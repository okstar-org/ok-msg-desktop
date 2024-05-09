#include "ContactWidget.h"
#include "ui_ContactWidget.h"

ContactWidget::ContactWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactWidget)
{
    ui->setupUi(this);
}

ContactWidget::~ContactWidget()
{
    delete ui;
}
