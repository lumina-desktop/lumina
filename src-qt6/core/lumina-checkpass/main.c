//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This function provides the basic current-user password validation
// The binary may need to have an effective root UID (setuid as root: "chmod 4555")
//   so that PAM can actually check the validity of the password.
//===========================================
//  SECURITY NOTE:
//    It is highly recomended that you have your PAM rules setup to disallow password checks for a time
//    after a number of failed attempts to prevent a user-level script from hammering this utility
//===========================================
//Standard C libary
#include <unistd.h>	// Standard C
#include <stdlib.h>
#include <stdio.h> 	// Usage output
#include <pwd.h>  		// User DB information
#include <string.h>
#include <sysexits.h>

//PAM/security libraries
#include <sys/types.h>
#include <security/pam_appl.h>

#define	AUTH_FAILED	1

//Non-interactive PAM conversation: answer any hidden-input prompt with the
// password passed in through appdata_ptr, ignore informational messages and
// refuse anything else (we cannot ask the user follow-up questions).
static int PAM_conv(
	int num_msg,
	const struct pam_message **msg,
	struct pam_response **resp,
	void *appdata_ptr)
{
	const char *pass = (const char*) appdata_ptr;
	if(num_msg <= 0 || resp == NULL){ return PAM_CONV_ERR; }
	struct pam_response *reply = calloc(num_msg, sizeof(struct pam_response));
	if(reply == NULL){ return PAM_BUF_ERR; }
	for(int i=0; i<num_msg; i++){
	  switch(msg[i]->msg_style){
	    case PAM_PROMPT_ECHO_OFF:
	      reply[i].resp = strdup(pass ? pass : "");
	      if(reply[i].resp == NULL){ goto fail; }
	      break;
	    case PAM_ERROR_MSG:
	    case PAM_TEXT_INFO:
	      break; //nothing to reply
	    default:
	      goto fail; //PAM_PROMPT_ECHO_ON or unknown: cannot answer
	  }
	}
	*resp = reply;
	return PAM_SUCCESS;
fail:
	for(int i=0; i<num_msg; i++){ free(reply[i].resp); }
	free(reply);
	return PAM_CONV_ERR;
}

//Pick the PAM service to authenticate against.
// BSDs ship /etc/pam.d/system. Linux distros differ: Arch/Fedora/Gentoo ship
// system-auth, Debian/Ubuntu/SUSE do not (they use common-auth, which is an
// include fragment without an account stack). "login" exists everywhere and is
// what other lockers such as i3lock use. A dedicated "lumina-checkpass" service
// file always wins if the system provides one.
static const char* PAM_service(){
#ifdef __linux__
	static const char *services[] = { "lumina-checkpass", "system-auth", "login", NULL };
	static const char *dirs[] = { "/etc/pam.d/", "/usr/lib/pam.d/", NULL };
	char path[256];
	for(int s=0; services[s]!=NULL; s++){
	  for(int d=0; dirs[d]!=NULL; d++){
	    snprintf(path, sizeof(path), "%s%s", dirs[d], services[s]);
	    if(access(path, R_OK)==0){ return services[s]; }
	  }
	}
	return "login";
#else
	return "system";
#endif
}

//Read one line (the password) from the stream, stripping the trailing newline.
// Returns NULL on read failure or an empty file.
static char* readPassword(FILE *fp){
	char *pass = NULL;
	size_t len = 0;
	ssize_t slen = getline(&pass, &len, fp);
	if(slen <= 0){ free(pass); return NULL; }
	if(pass[slen-1]=='\n'){ pass[slen-1] = '\0'; }
	return pass;
}
//-----


void showUsage(){
    fputs("lumina-checkpass: Simple user-level check for password validity (for screen unlockers and such).", stderr);
    fputs("Usage:", stderr);
    //fputs("  lumina-checkpass <password>", stderr);
    fputs("  lumina-checkpass -fd <file descriptor>", stderr);
    fputs("  lumina-checkpass -f <file path>", stderr);
    fputs("Returns: 0 for a valid password, 1 for invalid", stderr);
    exit(AUTH_FAILED);	// FIXME: Switch to EX_USAGE, or do callers depend on 1?
}

int main(int argc, char** argv){
  //Check the inputs
  if(argc!=3){
    //Invalid inputs - show the help text
    showUsage();
  }
  char *pass = NULL;
  if(argc==3 && 0==strcmp(argv[1],"-fd") ){
    // This replaces dangerous atoi(), which does no validation
    char *end;
    int fd = strtol(argv[2], &end, 10);
    if ( *end != '\0' )
    {
      fprintf(stderr, "Invalid file descriptor: %s\n", argv[2]);
      showUsage();
    }
    FILE *fp = fdopen(fd, "r");
    if(fp!=NULL){
      pass = readPassword(fp);
      fclose(fp);
    }
  }else if(argc==3 && 0==strcmp(argv[1],"-f") ){
    FILE *fp = fopen(argv[2], "r");
    if(fp!=NULL){
      pass = readPassword(fp);
      fclose(fp);
    }else{
      fprintf(stderr, "[ERROR] Could not open file: %s\n", argv[2]);
      fputs("----------------", stderr);
      showUsage();
    }
  }
  if(pass == NULL){ fputs("Could not read password!!", stderr); return AUTH_FAILED; } //error in reading password (or empty)
  //Only ever check the password of the user who invoked us (real UID - this binary may be setuid root).
  // getlogin() is not used: it returns NULL without a controlling tty/loginuid, and may name a different
  // user than the real UID (su sessions).
  struct passwd *pwd = getpwuid(getuid());
  if(pwd==NULL || pwd->pw_name==NULL){ return AUTH_FAILED; } //User could not be found in the database? (should never happen)
  char *cUser = strdup(pwd->pw_name); //copy: PAM modules may overwrite the static getpw* buffer
  if(cUser==NULL){ return AUTH_FAILED; }
  //Create the non-interactive PAM structures
  pam_handle_t *pamh;
  struct pam_conv pamc = { &PAM_conv, pass };
    int ret = pam_start( PAM_service(), cUser, &pamc, &pamh);
    if(ret != PAM_SUCCESS){ fputs("Could not initialize PAM", stderr); return AUTH_FAILED; } //could not init PAM
    //char* cPassword = argv[1];
    //Pre-seed the token where allowed (OpenPAM). Linux-PAM rejects PAM_AUTHTOK from applications
    // (PAM_BAD_ITEM), so there the password is supplied through PAM_conv instead - not an error.
    pam_set_item(pamh, PAM_AUTHTOK, pass);
    //Authenticate with PAM
    ret = pam_authenticate(pamh,0); //this can be true without verifying password if pam_self.so is used in the auth procedures (common)
    if( ret == PAM_SUCCESS ){ ret = pam_acct_mgmt(pamh,0); } //Check for valid, unexpired account and verify access restrictions
    //Stop the PAM instance
    pam_end(pamh,ret);
  //return verification result
  return ret == PAM_SUCCESS ? EX_OK : AUTH_FAILED;
}
