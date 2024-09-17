#include "reqrespwidget.h"



//Parametes ViewWidget
ReqRespWidget::ReqRespWidget(QWidget *parent)
    :LTableWidgetBox(parent, 1)
{

    this->setTitle("Last REQUEST-RESPONSE");
    QStringList headers;
    headers << "Request" << "Response";
    setHeaderLabels(headers, Qt::Horizontal);

    headers.clear();
    headers << "Transaction ID" << "Protocol ID" << "Protocol ID" << "Unit address" << "Function";
    setHeaderLabels(headers, Qt::Vertical);


    //vHeaderHide();

    this->resizeByContents();
}


