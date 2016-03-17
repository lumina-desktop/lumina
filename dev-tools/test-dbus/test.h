#include <QDialog>
#include <QDebug>

#include "ui_test.h"

namespace Ui{
	class Test;
};

class Test : public QDialog{
	Q_OBJECT
	
private:
	Ui::Test *ui;

private slots:
	void on_push_close_clicked(){ this->close(); }
	
public:
	Test() : QDialog(), ui(new Ui::Test){
	  qDebug() << "Creating Test Dialog:";
	  ui->setupUi(this);
	  qDebug() << "Loaded Dialog";
	}
	~Test(){ delete ui; }

};
