#include "TtyProcess.h"

TTYProcess::TTYProcess(QObject *parent) : QObject(parent){
  childProc = 0;
  sn = 0;
  ttyfd = 0;
}

TTYProcess::~TTYProcess(){
  closeTTY(); //make sure everything is closed properly
}
		
// === PUBLIC ===
bool TTYProcess::startTTY(QString prog, QStringList args){
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
  qDebug() << "Write:" << output;
  ::write(ttyfd, output.data(), output.size());
}

void TTYProcess::writeQtKey(int key){
  //Note that a shell does *not* take ANSI
    QByteArray ba;
    //Check for special keys
    switch(key){
      case Qt::Key_Backspace:
	//ba.append("^H/x1b[8p");    
        break;
      case Qt::Key_Left:
      case Qt::Key_Right:
        break;
      case Qt::Key_Up:
        //ba.append("\x1b[(224;72)p");
        break;
      case Qt::Key_Down:
        //ba.append("\x1b[(224;80)p");
        break;
  }
   
  //qDebug() << "Forward Input:" << txt << ev->key() << ba;
  if(!ba.isEmpty()){ this->writeTTY(ba); }
}

QByteArray TTYProcess::readTTY(){
  QByteArray BA;
  if(sn==0){ return BA; } //not setup yet
  char buffer[64];
  ssize_t rtot = read(sn->socket(),&buffer,64);
  buffer[rtot]='\0';
  BA = QByteArray(buffer, rtot);
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
    return BA;
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

  // COLOR CODES	
  index = raw.indexOf("\x1B[");
  while(index>=0){
    int end = raw.indexOf("m",index);
    if(end<0){ return raw; } //incomplete raw array
    raw = raw.remove(index, end-index+1);
    index = raw.indexOf("\x1B[");
  }
  //qDebug() << " - Removed Color Codes:" << raw;
  
  // SYSTEM BELL
  index = raw.indexOf("\x07");
  while(index>=0){ 
    //qDebug() << "Remove Bell:" << index;
    raw = raw.remove(index,1); 
    index = raw.indexOf("\x07");
  }
  //qDebug() << " - Fully Cleaned:" << raw;
  
  incomplete = false;
  return raw;
}

// === PRIVATE ===
pid_t TTYProcess::LaunchProcess(int& fd, char *prog, char **child_args){
  //Returns: -1 for errors, positive value (file descriptor) for the master side of the TTY to watch	

  //First open/setup a new pseudo-terminal file/device on the system (master side)
  fd = posix_openpt(O_RDWR); //open read/write
  if(fd<0){ return -1; } //could not create pseudo-terminal
  int rc = grantpt(fd); //set permissions
  if(rc!=0){ return -1; }
  rc = unlockpt(fd); //unlock file (ready for use)
  if(rc!=0){ return -1; }	
  //Now fork, return the Master device and setup the child
  pid_t PID = fork();
  if(PID==0){
    //SLAVE/child
    int fds = ::open(ptsname(fd), O_RDWR); //open slave side read/write
    ::close(fd); //close the master side from the slave thread
	  
    //Adjust the slave side mode to RAW
    struct termios TSET;
    rc = tcgetattr(fds, &TSET); //read the current settings
    cfmakeraw(&TSET); //set the RAW mode on the settings
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
