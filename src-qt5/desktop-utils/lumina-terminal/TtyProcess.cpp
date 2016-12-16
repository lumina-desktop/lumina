#include "TtyProcess.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QTimer>

#define DEBUG 1

TTYProcess::TTYProcess(QObject *parent) : QObject(parent){
  childProc = 0;
  sn = 0;
  ttyfd = 0;
  starting = true;
  fixReply = -1;
}

TTYProcess::~TTYProcess(){
  closeTTY(); //make sure everything is closed properly
}
		
// === PUBLIC ===
bool TTYProcess::startTTY(QString prog, QStringList args, QString workdir){
  if(workdir=="~"){ workdir = QDir::homePath(); }
  QDir::setCurrent(workdir);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  setenv("TERM","vt220-color",1);//"vt102-color",1); //vt100: VT100 emulation support (QTerminal sets "xterm" here)

  //setenv("TERMINFO","/etc/termcap",0);
  unsetenv("WINDOWID");
  //unsetenv("TERMCAP");
  //setenv("TERMCAP","/etc/termcap",1);
  //setenv("TERMCAP","vt220-color",1);
  //setenv("TERMCAP","vt102|vt220-color|dec vt102:' :do=^J:co#80:li#24:cl=50\E[;H\E[2J: :le=^H:bs:cm=5\E[%i%d;%dH:nd=2\E[C:up=2\E[A: :ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m: :md=2\E[1m:mr=2\E[7m:mb=2\E[5m:me=2\E[m:is=\E[1;24r\E[24;1H: :rs=\E>\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h:ks=\E[?1h\E=:ke=\E[?1l\E>: :ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:kb=^H: :ho=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:pt:sr=5\EM:vt#3: :sc=\E7:rc=\E8:cs=\E[%i%d;%dr:vs=\E[?7l:ve=\E[?7h: :mi:al=\E[L:dc=\E[P:dl=\E[M:ei=\E[4l:im=\E[4h:' vi $*",1);
  /*setenv("TERMCAP","'vt220-color' :do=2\E[B:co#80:li#24:cl=50\E[H\E[J:sf=2*\ED:\
	:le=^H:bs:am:cm=5\E[%i%d;%dH:nd=2\E[C:up=2\E[A:\
	:ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m:\
	:md=2\E[1m:mr=2\E[7m:mb=2\E[5m:me=2\E[m:\
	:is=\E>\E[?1;3;4;5l\E[?7;8h\E[1;24r\E[24;1H:\
	:if=/usr/share/tabset/vt100:nw=2\EE:ho=\E[H:\
	:as=2\E(0:ae=2\E(B:\
	:ac=``aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||:\
	:rs=\E>\E[?1;3;4;5l\E[?7;8h:ks=\E[?1h\E=:ke=\E[?1l\E>:\
	:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:kb=\177:\
	:k0=\EOy:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:k5=\EOt:\
	:k6=\EOu:k7=\EOv:k8=\EOl:k9=\EOw:k;=\EOx:@8=\EOM:\
	:K1=\EOq:K2=\EOr:K3=\EOs:K4=\EOp:K5=\EOn:pt:sr=2*\EM:xn:\
	:sc=2\E7:rc=2\E8:cs=5\E[%i%d;%dr:UP=2\E[%dA:DO=2\E[%dB:RI=2\E[%dC:\
	:LE=2\E[%dD:ct=2\E[3g:st=2\EH:ta=^I:ms:bl=^G:cr=^M:eo:it#8:\
	:RA=\E[?7l:SA=\E[?7h:po=\E[5i:pf=\E[4i:",1); //see /etc/termcap as well */
  QStringList filter = env.keys().filter("XTERM");
  for(int i=0; i<filter.length(); i++){ unsetenv(filter[i].toLocal8Bit().data()); }
  //if(env.contains("TERM")){ unsetenv("TERM"); }
  //else if(env.contains
  //Turn the program/arguments into C-compatible arrays
  char cprog[prog.length()]; strcpy(cprog, prog.toLocal8Bit().data());
  char *cargs[args.length()+2];
  QByteArray nullarray;
  for(int i=0; i<args.length()+2; i++){
    // First arg needs to be the program
    if ( i == 0 ) {
      cargs[i] = new char[ prog.toLocal8Bit().size()+1];
      strcpy( cargs[i], prog.toLocal8Bit().data() );
    } else if(i<args.length()){
      cargs[i] = new char[ args[i].toLocal8Bit().size()+1];
      strcpy( cargs[i], args[i].toLocal8Bit().data() );
    }else{
      cargs[i] = NULL;
    }
  }
  if(DEBUG){ qDebug() << "PTY Start:" << prog; }
  //Launch the process attached to a new PTY
  int FD = 0;
  pid_t tmp = LaunchProcess(FD, cprog, cargs);
  if(DEBUG){ 
    qDebug() << " - PID:" << tmp; 
    qDebug() << " - FD:" << FD; 
  }
  if(tmp<0){ return false; } //error
  else{
    childProc = tmp;
    //Load the file for close notifications
      //TO-DO
    //Watch the socket for activity
    sn= new QSocketNotifier(FD, QSocketNotifier::Read);
	sn->setEnabled(true);
	connect(sn, SIGNAL(activated(int)), this, SLOT(checkStatus(int)) );
    ttyfd = FD;
   if(DEBUG){ qDebug() << " - PTY:" << ptsname(FD); }
    starting = true;
    return true;
  }
}

void TTYProcess::closeTTY(){
  int junk;
  if(0==waitpid(childProc, &junk, WNOHANG)){
    kill(childProc, SIGKILL);	
  }
  if(ttyfd!=0 && sn!=0){
    sn->setEnabled(false);
    ::close(ttyfd);
    ttyfd = 0;
    emit processClosed();
  }
}

void TTYProcess::writeTTY(QByteArray output){
  //qDebug() << "Write:" << output;
  static QList<QByteArray> knownFixes;
  if(knownFixes.isEmpty()){ knownFixes << "\x1b[C" << "\x1b[D" << "\b" << "\x7F" << "\x08"; }//<<"\x1b[H"<<"\x1b[F"; }
  fixReply = knownFixes.indexOf(output);
  ::write(ttyfd, output.data(), output.size());
}

QByteArray TTYProcess::readTTY(){
  QByteArray BA;
  //qDebug() << "Read TTY";
  if(sn==0){ return BA; } //not setup yet
  char buffer[64];
  ssize_t rtot = read(sn->socket(),&buffer,64);
  //buffer[rtot]='\0';
  BA = QByteArray(buffer, rtot);
  //qDebug() << " - Got Data:" << BA;
  if(!fragBA.isEmpty()){
    //Have a leftover fragment, include this too
    BA = BA.prepend(fragBA);
    fragBA.clear();
  }
  bool bad = true;
  BA = CleanANSI(BA, bad);
  if(bad){
    //incomplete fragent - read some more first
    fragBA = BA; 
    return readTTY();
  }else{
    if(DEBUG){ qDebug() << "Read Data:" << BA; }
    //BUG BYPASS - 12/7/16
    //If the PTY gets input fairly soon after starting, the PTY will re-print the initial line(s)
    if(starting && !BA.contains("\n") ){
      //qDebug() << "Starting phase 1:" << BA;
       writeTTY("tset\n"); //Terminal Setup utility (uses the TERM env variable)
      BA.clear();
    }else if(starting){
      //qDebug() << "Starting phase 2:" << BA;
      BA.remove(0, BA.indexOf("\n")+1);
      starting = false;
    }
    //Apply known fixes for replies to particular inputs (mostly related to cursor position *within* the current line)
    // This appears to be primarily from the concept that the cursor position is always at the end of the line (old VT limitation?)
    //  so almost all these fixes are for cursor positioning within the current line
    if(fixReply >= 0){
      if(DEBUG){ qDebug() << "Fix Reply:" <<fixReply <<  BA; }
      switch(fixReply){
	case 0: //Right arrow ("\x1b[C") - PTY reply re-prints the next character rather than moving the cursor
          if(BA.length()>0){
            BA.remove(0,1);
            BA.prepend("\x1b[C"); //just move the cursor - don't re-print that 1 character
          }
	  break;
	case 1: //Left arrow ("\x1b[D") - PTY erases the previous character instead of moving the cursor
          if(BA.startsWith("\b")){
            BA.remove(0,1);
            BA.prepend("\x1b[D"); //just move the cursor - don't send the "back" character (\b)
          }
	  break;
	case 2: //Backspace or delete - PTY works fine if on the end of the line, but when in the middle of a line it will backpace a number of times after clearing (same as left arrow issue)
        case 3:
	case 4:
          if(BA.contains("\x1b[K")){
	    while(BA.indexOf("\x1b[K") < BA.lastIndexOf("\b") ){
              BA.replace( BA.lastIndexOf("\b"), 1, "\x1b[D"); //just move the cursor left - don't send the "back" character (\b)
            }
          }
	  break;
	case 5: //Home Key
          BA = "\x1b[H";
          break;
	case 6: //End Key
          BA = "\x1b[F";
          break;
      }
      fixReply = -1; //done with the fix - resume normal operations
      if(DEBUG){ qDebug() << " - Fixed:" << BA; }
    }
    return BA;
  }
}

void TTYProcess::setTerminalSize(QSize chars, QSize pixels){
  if(ttyfd==0){ return; }
  struct winsize c_sz;
    c_sz.ws_row = chars.height();
    c_sz.ws_col = chars.width();
    c_sz.ws_xpixel = pixels.width();
    c_sz.ws_ypixel = pixels.height();
  if( ioctl(ttyfd, TIOCSWINSZ, &ws) ){
    qDebug() << "Error settings terminal size";
  }else{
    //qDebug() <<"Set Terminal Size:" << pixels << chars;
  }
}

bool TTYProcess::isOpen(){
  return (ttyfd!=0);
}

QByteArray TTYProcess::CleanANSI(QByteArray raw, bool &incomplete){
  incomplete = true;
  //qDebug() << "Clean ANSI Data:" << raw;
  //IN_LINE TERMINAL COLOR CODES (ANSI Escape Codes) "\x1B[<colorcode>m"
  //  - Just remove them for now
	
  //Special XTERM encoding (legacy support)
  int index = raw.indexOf("\x1B]");
  while(index>=0){
    //The end character of this sequence is the Bell command ("\x07")
    int end = raw.indexOf("\x07");
    if(end<0){ return raw; } //incomplete raw array
    raw = raw.remove(index, end-index+1);
    index = raw.indexOf("\x1B]");
  }

  // GENERIC ANSI CODES ((Make sure the output is not cut off in the middle of a code)
  index = raw.indexOf("\x1B");
  while(index>=0){
    //CURSOR MOVEMENT
    int end = 0;
    for(int i=1; i<raw.size() && end==0; i++){
      if(raw.size() < index+i){ return raw; }//cut off - go back for more data
      //qDebug() << "Check Char:" << raw.at(index+i);
      if( QChar(raw.at(index+i)).isLetter() ){
        end = i; //found the end of the control code
      }
    }
    index = raw.indexOf("\x1B",index+1); //now find the next one
  }
  
  // SYSTEM BELL
  index = raw.indexOf("\x07");
  while(index>=0){ 
    //qDebug() << "Remove Bell:" << index;
    raw = raw.remove(index,1); 
    index = raw.indexOf("\x07");
  }
  //VT220(?) print character code (cut out the code, leave the character)
  index=raw.indexOf("\x1b[@");
  while(index>=0){ 
    raw = raw.remove(index,3); 
    index = raw.indexOf("\x1b[@");
  }

  //VT102 Identify request
  index = raw.indexOf("\x1b[Z");
  while(index>=0){ 
    raw = raw.remove(index,3); 
    index = raw.indexOf("\x1b[Z");
    //Also send the proper reply to this identify request right away
    writeTTY("\x1b[/Z");
  }
 //Terminal Status request
  index = raw.indexOf("\x1b[5n");
  while(index>=0){ 
    raw = raw.remove(index,4); 
    index = raw.indexOf("\x1b[5n");
    //Also send the proper reply to this identify request right away
    writeTTY("\x1b[c"); //everything ok
   }
  //Terminal Identify request
  index = raw.indexOf("\x1b[c");
  while(index>=0){ 
    raw = raw.remove(index,3); 
    index = raw.indexOf("\x1b[c");
    //Also send the proper reply to this identify request right away
    writeTTY("\x1b[1c"); //VT220 reply code
  }
  //Terminal Identify request (xterm/termcap?)
  /*index = raw.indexOf("\x1b[P");
  while(index>=0){ 
    raw = raw.remove(index,3); 
    index = raw.indexOf("\x1b[P");
    //Also send the proper reply to this identify request right away
    qDebug() << " - Got XTERM/TERMCAP identify request ([P)";
    writeTTY("\x1b[/Z");
  }*/

  incomplete = false;
  return raw;
}

// === PRIVATE ===
pid_t TTYProcess::LaunchProcess(int& fd, char *prog, char **child_args){
  //Returns: -1 for errors, positive value (file descriptor) for the master side of the TTY to watch	
  //First open/setup a new pseudo-terminal file/device on the system (master side)
  fd = posix_openpt(O_RDWR | O_NOCTTY); //open read/write
  if(fd<0){ return -1; } //could not create pseudo-terminal
  int rc = grantpt(fd); //set permissions
  if(rc!=0){ return -1; }
  rc = unlockpt(fd); //unlock file (ready for use)
  //rc = fchown(fd, getuid(), getgid());
  setupTtyFd(fd);
  if(rc!=0){ return -1; }	
  //Now fork, return the Master device and setup the child
  pid_t PID = fork();
  if(PID==0){
    //SLAVE/child
    int fds = ::open(ptsname(fd), O_RDWR | O_NOCTTY); //open slave side read/write
    rc = fchown(fds, getuid(), getgid());
    ::close(fd); //close the master side from the slave thread
	  
    //Adjust the slave side mode to SANE
    setupTtyFd(fds);
    //Change the controlling terminal in child thread to the slave PTY
    ::close(0); //close current terminal standard input
    ::close(1); //close current terminal standard output
    ::close(2); //close current terminal standard error
    dup(fds); // Set slave PTY as standard input (0);
    dup(fds); // Set slave PTY as standard output (1);
    dup(fds); // Set slave PTY as standard error (2);
	  
    setsid();  //Make current process new session leader (so we can set controlling terminal)
    ioctl(0,TIOCSCTTY, 1); //Set the controlling terminal to the slave PTY
	  
    //Execute the designated program
    //rc = execvp("tset", NULL);
    rc = execvp(prog, child_args);
    ::close(fds); //no need to keep original file descriptor open any more
    exit(rc);
  }
  //MASTER thread (or error)
  return PID;
}

void TTYProcess::setupTtyFd(int fd){
  struct termios TSET;
    tcgetattr(fd, &TSET); //read the current settings
    cfmakesane(&TSET); //set the SANE mode on the settings ( RAW: cfmakeraw(&TSET); )
    //Set Input Modes
    //TSET.c_iflag |= IGNPAR; //ignore parity errors
    //TSET.c_iflag &= ~(IGNBRK | PARMRK | ISTRIP | ICRNL | IXON | IXANY | IXOFF); //ignore special characters
    //TSET.c_iflag &= IUTF8; //enable UTF-8 support
    //Set Local Modes
    //TSET.c_lflag &= (ECHO | ECHONL | ECHOKE); //Echo inputs (normal, newline, and KILL character line break)
    //TSET.c_lflag &= ~ICANON ;  //non-canonical mode (individual inputs - not a line-at-a-time)
    //Set Control Modes
    //TSET.c_cflag |= CLOCAL; //Local Terminal Connection (non-modem)
    //TSET.c_lflag &= ~IEXTEN;
    //TSET.c_cflag &= ~(CSIZE | PARENB);
    //TSET.c_cflag |= CS8;
    //tt.c_oflag &= ~OPOST; // disable special output processing
    //Set Output Modes
    //TSET.c_oflag |= OPOST;
    //TSET.c_oflag |= OXTABS;
    TSET.c_cc[VTIME] = 0; // timeout
    //Now apply the settings
    tcsetattr(fd, TCSANOW, &TSET); //apply the changed settings
}

// === PRIVATE SLOTS ===
void TTYProcess::checkStatus(int sock){
  //This is run when the socket gets activated
  if(sock!=ttyfd){
	  
  }
  //Make sure the child PID is still active
  int junk;
  if(0!=waitpid(childProc, &junk, WNOHANG)){
    this->closeTTY(); //clean up everything else
  }else{
    emit readyRead();
  }
}
