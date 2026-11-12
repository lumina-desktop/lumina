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

//Found this little snippet from SDDM - nice alternative to using the entire openpam library from FreeBSD
static int PAM_conv(
	int num_msg,
	const struct pam_message **msg,
	struct pam_response **resp,
	void *ctx)
{
	return PAM_SUCCESS;
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
    size_t len;
    if(fp!=NULL){
      ssize_t slen = getline(&pass, &len, fp);
      if(pass[slen-1]=='\n'){ pass[slen-1] = '\0'; }
    }
    fclose(fp);
  }else if(argc==3 && 0==strcmp(argv[1],"-f") ){
    FILE *fp = fopen(argv[2], "r");
    size_t len;
    if(fp!=NULL){
      ssize_t slen = getline(&pass, &len, fp);
      if(pass[slen-1]=='\n'){ pass[slen-1] = '\0'; }
    }else{
      fputs("[ERROR] Unknown option provided", stderr);
      fputs("----------------", stderr);
      showUsage();
    }
    fclose(fp);
  }
  if(pass == NULL){ fputs("Could not read password!!", stderr); return AUTH_FAILED; } //error in reading password
  //puts("Read Password:");
  //puts(pass);
  //Validate current user (make sure current UID matches the logged-in user,
  char* cUser = getlogin();
  struct passwd *pwd = NULL;
  pwd = getpwnam(cUser);
  if(pwd==NULL){ return AUTH_FAILED; } //Login user could not be found in the database? (should never happen)
  if( getuid() != pwd->pw_uid ){ return AUTH_FAILED; } //Current UID does not match currently logged-in user UID
  //Create the non-interactive PAM structures
  pam_handle_t *pamh;
  struct pam_conv pamc = { &PAM_conv, 0 };
    //Place the user-supplied password into the structure
#ifdef __linux__
    int ret = pam_start( "system", cUser, &pamc, &pamh);
#else
    int ret = pam_start( "system-auth", cUser, &pamc, &pamh);
#endif
    if(ret != PAM_SUCCESS){ fputs("Could not initialize PAM", stderr); return AUTH_FAILED; } //could not init PAM
    //char* cPassword = argv[1];
    ret = pam_set_item(pamh, PAM_AUTHTOK, pass);
    if(ret != PAM_SUCCESS){ fputs("Could not set conversation structure", stderr); }
    //Authenticate with PAM
    ret = pam_authenticate(pamh,0); //this can be true without verifying password if pam_self.so is used in the auth procedures (common)
    if( ret == PAM_SUCCESS ){ ret = pam_acct_mgmt(pamh,0); } //Check for valid, unexpired account and verify access restrictions
    //Stop the PAM instance
    pam_end(pamh,ret);
  //return verification result
  return ret == PAM_SUCCESS ? EX_OK : AUTH_FAILED;
}
