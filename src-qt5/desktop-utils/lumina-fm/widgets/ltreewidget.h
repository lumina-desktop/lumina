class LTreeWidget : public QTreeWidget{
Q_OBJECT

public:

LTreeWidget(QWidget* parent = 0) : QTreeWidget(parent)
{
setSortingEnabled(false); // disbale the default built in sorting

// our sorting method 
header()->setSortIndicatorShown(true);
header()->setClickable(true);
connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(customSortByColumn(int)));
customSortByColumn(header()->sortIndicatorSection());
}

public slots:
void customSortByColumn(int column)
{
Qt::SortOrder order = header()->sortIndicatorOrder(); // get the order
sortItems(column, order);
}
};
