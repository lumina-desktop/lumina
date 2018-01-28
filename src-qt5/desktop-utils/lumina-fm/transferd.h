class transactionInfo{
public:
  QString filepath;
  qint64 filesize;
  Bool finished;
};


QStringList sel;
int totaltransactions currentTransfered;
qint64 totalsize, currentTransferedSize, transferTime, avgTransferRate;
bool paused;
QElapsedTimer timer;
