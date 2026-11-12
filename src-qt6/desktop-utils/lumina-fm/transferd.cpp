

int totaltransactions = sel.length();
QList<TransactionInfo> transaction;
for(int i=0;i<sel.length();i++){
    QStringList info;
    info << sel[i] << sel[i].size() << false;
    totalsize = totalsize + sel[i].size();
}
bool paused = false;
for(int i=0; i<transaction.length(); i++){

while(paused = 1 ){
  pausetimer = new QTimer(this);
  pausetimer->start(5000);
}
QElapsedTimer timer;
timer.start();
//copy/move file
timer.elapsed(); // "milliseconds"
transferTime = transferTime + timer.elapsed()
transaction[i].value(2) = true;
currentTransfered++;
currentTransferedSize = currentTransferedSize + transaction[i].value(1);
//calculate transfer data rate from transferTime and currentTransferedSize
avgTransferRate = ( currentTransferedSize / (transferTime * 1000) )
//can do estimates on potential time?
}
