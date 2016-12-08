#include "TtyProcess.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QTimer>

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
  unsetenv("TERMCAP");
  //setenv("TERMCAP","vt102-color",1);
  /*setenv("TERMCAP",":do=2\E[B:co#80:li#24:cl=50\E[H\E[J:sf=2*\ED:\
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
	:RA=\E[?7l:SA=\E[?7h:po=\E[5i:pf=\E[4i:",1); //see /etc/termcap as well*/
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
  qDebug() << "PTY Start:" << prog;
  //Launch the process attached to a new PTY
  int FD = 0;
  pid_t tmp = LaunchProcess(FD, cprog, cargs);
  qDebug() << " - PID:" << tmp;
  qDebug() << " - FD:" << FD;
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
    qDebug() << " - PTY:" << ptsname(FD);
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
  if(knownFixes.isEmpty()){ knownFixes << "\x1b[C" << "\x1b[D"; }
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
    qDebug() << "Read Data:" << BA;
    //BUG BYPASS - 12/7/16
    //If the PTY gets input fairly soon after starting, the PTY will re-print the initial line(s)
    if(starting && !BA.contains("\n") ){
      //qDebug() << "Starting phase 1:" << BA;
       writeTTY("\n\b"); //newline + backspace
      BA.clear();
    }else if(starting){
      //qDebug() << "Starting phase 2:" << BA;
      BA.remove(0, BA.indexOf("\n")+1);
      starting = false;
    }
    //Apply known fixes for replies to particular inputs
    if(fixReply >= 0){
      qDebug() << "Fix Reply:" <<fixReply <<  BA;
      switch(fixReply){
	case 0: //Right arrow ("\x1b[C") - PTY reply clears the screen after printing only the first char (not everything)
          if(BA.length()>0){
            BA.remove(0,1);
            BA.prepend("\x1b[C"); //just move the cursor - don't re-print that 1 character
          }
	  break;
	case 1: //Right arrow ("\x1b[D") - PTY reply clears the screen after printing only the first char (not everything)
          if(BA.startsWith("\b")){
            BA.remove(0,1);
            BA.prepend("\x1b[D"); //just move the cursor - don't send the "back" character (\b)
          }
	  break;
      }
      fixReply = -1; //done with the fix - resume normal operations
      //qDebug() << " - Fixed:" << BA;
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

  //VT102 Identify request
  index = raw.indexOf("\x1b[Z");
  while(index>=0){ 
    raw = raw.remove(index,1); 
    index = raw.indexOf("\x1b[Z");
    //Also send the proper reply to this identify request right away
    writeTTY("\x1b[/Z");
  }
    
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
  if(rc!=0){ return -1; }	
  //Now fork, return the Master device and setup the child
  pid_t PID = fork();
  if(PID==0){
    //SLAVE/child
    int fds = ::open(ptsname(fd), O_RDWR | O_NOCTTY); //open slave side read/write
    ::close(fd); //close the master side from the slave thread
	  
    //Adjust the slave side mode to RAW
    struct termios TSET;
    rc = tcgetattr(fds, &TSET); //read the current settings
    cfmakesane(&TSET); //set the RAW mode on the settings ( cfmakeraw(&TSET); )
    tcsetattr(fds, TCSANOW, &TSET); //apply the changed settings

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
    rc = execvp(prog, child_args);
    ::close(fds); //no need to keep original file descriptor open any more
    exit(rc);
  }
  //MASTER thread (or error)
  return PID;
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
