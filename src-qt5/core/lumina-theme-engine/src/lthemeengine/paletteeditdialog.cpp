#include <QPalette>
#include <QColorDialog>
#include <QSettings>
#include "lthemeengine.h"
#include "paletteeditdialog.h"
#include "ui_paletteeditdialog.h"

PaletteEditDialog::PaletteEditDialog(const QPalette &palette, QStyle *currentStyle, QWidget *parent) : QDialog(parent), m_ui(new Ui::PaletteEditDialog){
  m_currentStyle = currentStyle;
  m_ui->setupUi(this);
  m_ui->tableWidget->setColumnCount(3);
  m_ui->tableWidget->setRowCount(QPalette::NColorRoles);
  m_ui->tableWidget->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing() + 10);
  m_ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QStringList labels;
  labels << tr("Active") << tr("Inactive") << tr("Disabled");
  m_ui->tableWidget->setHorizontalHeaderLabels(labels);
  setPalette(palette);
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  restoreGeometry(settings.value("PaletteEditor/geometry").toByteArray());
}

PaletteEditDialog::~PaletteEditDialog(){
  delete m_ui;
}

QPalette PaletteEditDialog::selectedPalette() const{
  QPalette palette;
  for(int i = 0; i < QPalette::NColorRoles; i++){
    palette.setBrush(QPalette::Active, QPalette::ColorRole(i), m_ui->tableWidget->item(i,0)->background().color());
    palette.setBrush(QPalette::Inactive, QPalette::ColorRole(i), m_ui->tableWidget->item(i,1)->background().color());
    palette.setBrush(QPalette::Disabled, QPalette::ColorRole(i), m_ui->tableWidget->item(i,2)->background().color());
    }
  return palette;
}

void PaletteEditDialog::setPalette(const QPalette &palette){
  for(int i = 0; i < QPalette::NColorRoles; i++){
    if(!m_ui->tableWidget->item(i,0)){ m_ui->tableWidget->setItem(i, 0, new QTableWidgetItem()); }
    if(!m_ui->tableWidget->item(i,1)){ m_ui->tableWidget->setItem(i, 1, new QTableWidgetItem()); }
    if(!m_ui->tableWidget->item(i,2)){ m_ui->tableWidget->setItem(i, 2, new QTableWidgetItem()); }
    m_ui->tableWidget->item(i,0)->setBackground(QBrush(palette.color(QPalette::Active, QPalette::ColorRole(i))));
    m_ui->tableWidget->item(i,1)->setBackground(QBrush(palette.color(QPalette::Inactive, QPalette::ColorRole(i))));
    m_ui->tableWidget->item(i,2)->setBackground(QBrush(palette.color(QPalette::Disabled, QPalette::ColorRole(i))));
    }
  QStringList labels;
  labels << tr("Window text") << tr("Button background") << tr("Bright") << tr("Less bright") << tr("Dark") << tr("Less dark") << tr("Normal text") << tr("Bright text") << tr("Button text") << tr("Normal background") << tr("Window") << tr("Shadow") << tr("Highlight") << tr("Highlighted text")  << tr("Link")  << tr("Visited link") << tr("Alternate background") << tr("Default") << tr("Tooltip background")  << tr("Tooltip text") << tr("Placeholder text");
  m_ui->tableWidget->setVerticalHeaderLabels(labels);
}

void PaletteEditDialog::hideEvent(QHideEvent *){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.setValue("PaletteEditor/geometry", saveGeometry());
}

void PaletteEditDialog::on_tableWidget_itemClicked(QTableWidgetItem *item){
  QColor color = QColorDialog::getColor(item->background().color(), this, tr("Select Color"));
  if(color.isValid()){
    item->setBackground(QBrush(color));
    emit paletteChanged(selectedPalette());
    }
}

void PaletteEditDialog::on_resetPaletteButton_clicked(){
  setPalette(m_currentStyle->standardPalette());
  emit paletteChanged(selectedPalette());
}

void PaletteEditDialog::on_buildInactiveButton_clicked(){
  QPalette palette = selectedPalette();
  for(int i = 0; i < QPalette::NColorRoles; i++){
    palette.setColor(QPalette::Inactive, QPalette::ColorRole(i),
    palette.color(QPalette::Active, QPalette::ColorRole(i)));
    }
  setPalette(palette);
  emit paletteChanged(selectedPalette());
}

void PaletteEditDialog::on_buildDisabledButton_clicked(){
  QPalette palette = selectedPalette();
  for(int i = 0; i < QPalette::NColorRoles; i++){
    palette.setColor(QPalette::Disabled, QPalette::ColorRole(i),
    palette.color(QPalette::Active, QPalette::ColorRole(i)));
    }
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
  palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGray);
  palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
  palette.setColor(QPalette::Disabled, QPalette::HighlightedText, Qt::darkGray);
  setPalette(palette);
  emit paletteChanged(selectedPalette());
}
