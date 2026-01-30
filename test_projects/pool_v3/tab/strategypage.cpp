#include "strategypage.h"
#include "strategydata.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "ltable.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QSplitter>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QFrame>
#include <QLineEdit>
#include <QToolButton>
#include <QDir>


#define DT_LINE_ENABLED_COLOR       QString("#F5F5DC")
#define DT_LINE_DISABLED_COLOR       QString("#D3D3D3")

#define FULL_LIQ_SIZE_KEY       QString("liq")
#define RANGE_WIDTH_KEY         QString("rwidth")
#define PRIOR_TOKEN_PART_KEY    QString("pt_part")


// DefiStrategyPage
DefiStrategyPage::DefiStrategyPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkStrategy),
      m_strategyCombo(NULL),
      m_poolCombo(NULL),
      m_startTimeEdit(NULL),
      m_lineResultEdit(NULL),
      m_dataObj(NULL)
{
    setObjectName("defi_strategy_tab_page");

    //init table
    initPageBoxes();
    controlButtonsDisable();


}
void DefiStrategyPage::controlButtonsDisable()
{
    qDebug("DefiStrategyPage::controlButtonsDisable()");
    startLineBtn()->setEnabled(false);
    stopLineBtn()->setEnabled(false);
    closeStepBtn()->setEnabled(false);
    nextStepBtn()->setEnabled(false);
}
void DefiStrategyPage::resetStartParamsControls()
{
    qDebug("DefiStrategyPage::resetStartParamsControls()");
    QLineEdit *edit_liq = qobject_cast<QLineEdit*>(m_controls.value(FULL_LIQ_SIZE_KEY));
    if (edit_liq) {edit_liq->setEnabled(true); edit_liq->setText("---");}
    QLineEdit *edit_range = qobject_cast<QLineEdit*>(m_controls.value(RANGE_WIDTH_KEY));
    if (edit_range) {edit_range->setEnabled(true); edit_range->setText("---");}
    QComboBox *combo_part = qobject_cast<QComboBox*>(m_controls.value(PRIOR_TOKEN_PART_KEY));
    if (combo_part) {combo_part->setEnabled(true); combo_part->setCurrentIndex(2);}

    m_startTimeEdit->setStyleSheet(QString("background-color: %1").arg(DT_LINE_DISABLED_COLOR));
    m_startTimeEdit->setText(QString());
}
void DefiStrategyPage::restoreStartParamsByLine(const StrategyLineData *line)
{
    qDebug()<<QString("DefiStrategyPage::restoreStartParamsByLine()  pool[%1]").arg(line->pool_addr);
    const StrategyLineParameters& sp = line->start_parameters;
    QLineEdit *edit_liq = qobject_cast<QLineEdit*>(m_controls.value(FULL_LIQ_SIZE_KEY));
    if (edit_liq) {edit_liq->setEnabled(false); edit_liq->setText(QString::number(sp.liq_size));}
    QLineEdit *edit_range = qobject_cast<QLineEdit*>(m_controls.value(RANGE_WIDTH_KEY));
    if (edit_range) {edit_range->setEnabled(false); edit_range->setText(QString::number(sp.range_width));}
    QComboBox *combo_part = qobject_cast<QComboBox*>(m_controls.value(PRIOR_TOKEN_PART_KEY));
    if (combo_part)
    {
        combo_part->setEnabled(false);
        int i = combo_part->findText(QString("%1 %").arg(sp.prior_asset_size));
        if (i < 0) i = 2;
        combo_part->setCurrentIndex(i);
    }

    m_startTimeEdit->setStyleSheet(QString("background-color: %1").arg(DT_LINE_ENABLED_COLOR));
    m_startTimeEdit->setText(DefiStrategyData::fromTsPointToStr(line->ts_open));

}
void DefiStrategyPage::updateControlButtonsState(int line_index)
{
    qDebug()<<QString("DefiStrategyPage::updateControlButtonsState() line_index=%1").arg(line_index);
    controlButtonsDisable();
    resetStartParamsControls();
    if (line_index < 0)
    {
        startLineBtn()->setEnabled(true);
        return;
    }

    const StrategyLineData *line = m_dataObj->lineAt(line_index);
    if (line)
    {
        if (line->isFinished()) {qDebug("line is finished"); return;}
        restoreStartParamsByLine(line);

        if (line->lastStepOpened())
        {
            closeStepBtn()->setEnabled(true);
        }
        else
        {
            stopLineBtn()->setEnabled(true);
            nextStepBtn()->setEnabled(true);
        }
    }
}
void DefiStrategyPage::initDataObj()
{
    m_dataObj = new DefiStrategyData(curChainName(), this);
    connect(m_dataObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_dataObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

    bool ok;
    m_dataObj->loadStrategyFile(ok);
    if (!ok)
    {
        m_strategyCombo->setEnabled(false);
        m_poolCombo->setEnabled(false);
        foreach (QWidget *w, m_controls) w->setEnabled(false);

        delete m_dataObj;
        m_dataObj = NULL;
        return;
    }

    qDebug()<<QString("loaded lines %1").arg(m_dataObj->lineCount());
    slotPoolChanged();
}
void DefiStrategyPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);

    m_table->resizeByContents();
    slotUpdateComboPools();

    initDataObj();
}
void DefiStrategyPage::initPageBoxes()
{
    // init table
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Current game state");
    m_table->vHeaderHide();

    QStringList headers;
    headers << "Step" << "Open date" << "Exit date" << "Price range" << "Deposited" << "Current assets" << "Rewards" << "Step result";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    //m_table->setSelectionColor("#BFBFBF");
    h_splitter->addWidget(m_table);

    // init control box
    QGroupBox *box = new QGroupBox("Control", this);
    h_splitter->addWidget(box);
    if (box->layout()) delete box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    box->setLayout(g_lay);

    int lay_row = 0;
    g_lay->addWidget(new QLabel("Strategy", this), lay_row, 0);
    m_strategyCombo = new QComboBox(this);
    g_lay->addWidget(m_strategyCombo, lay_row, 1);
    lay_row++;
    g_lay->addWidget(new QLabel("Pool", this), lay_row, 0);
    m_poolCombo = new QComboBox(this);
    g_lay->addWidget(m_poolCombo, lay_row, 1);
    lay_row++;
    g_lay->addWidget(new QLabel("Start line time", this), lay_row, 0);
    m_startTimeEdit = new QLineEdit(this);
    m_startTimeEdit->setReadOnly(true);
    m_startTimeEdit->setText(QString());
    m_startTimeEdit->setStyleSheet(QString("background-color: %1").arg(DT_LINE_DISABLED_COLOR));
    g_lay->addWidget(m_startTimeEdit, lay_row, 1);
    lay_row++;

    m_strategyCombo->clear();
    m_strategyCombo->addItem("Follow price", dstFollowPrice);
    m_strategyCombo->addItem("Martingale", dstMartingale);
    m_strategyCombo->addItem("Stable", dstStable);

    //add line separator
    QFrame* myFrame = new QFrame(this);
    myFrame->setFrameShape(QFrame::HLine);
    myFrame->setLineWidth(1);
    myFrame->setStyleSheet(QString("QFrame {color: #000050;}"));
    g_lay->addWidget(myFrame, lay_row, 0, 1, 2);
    lay_row++;

    /////////////////////////////////////////////////////////////////

    //controls line settings
    g_lay->addWidget(new QLabel("Liquidity size", this), lay_row, 0);
    QLineEdit *liq_edit = new QLineEdit(this);
    liq_edit->setObjectName("liq_edit");
    liq_edit->setText("2.5");
    g_lay->addWidget(liq_edit, lay_row, 1);
    m_controls.insert(FULL_LIQ_SIZE_KEY, liq_edit);
    lay_row++;

    g_lay->addWidget(new QLabel("Price range width   ", this), lay_row, 0);
    QLineEdit *rsize_edit = new QLineEdit(this);
    rsize_edit->setObjectName("rwidth_edit");
    rsize_edit->setText("0.3");
    g_lay->addWidget(rsize_edit, lay_row, 1);
    m_controls.insert(RANGE_WIDTH_KEY, rsize_edit);
    lay_row++;

    g_lay->addWidget(new QLabel("Prior asset size", this), lay_row, 0);
    QComboBox *asize_combo = new QComboBox(this);
    asize_combo->setObjectName("asize_combo");
    asize_combo->clear();
    for (int i=3; i<=7; i++)
        asize_combo->addItem(QString("%1 %").arg(i*10));
    g_lay->addWidget(asize_combo, lay_row, 1);
    m_controls.insert(PRIOR_TOKEN_PART_KEY, asize_combo);
    lay_row++;

    ////////////////////////////////////////////////
    g_lay->addWidget(new QLabel("Total line result", this), lay_row, 0);
    m_lineResultEdit = new QLineEdit(this);
    m_lineResultEdit->setReadOnly(true);
    m_lineResultEdit->setStyleSheet(QString("background-color: %1").arg(DT_LINE_DISABLED_COLOR));
    g_lay->addWidget(m_lineResultEdit, lay_row, 1);
    lay_row++;


    // add spacer
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Expanding);
    g_lay->addItem(verticalSpacer, lay_row, 0);
    lay_row++;


    // control buttons
    QFrame *b_frame = new QFrame(this);
    b_frame->setFrameStyle(QFrame::Panel);
    if (b_frame->layout()) delete b_frame->layout();
    b_frame->setLayout(new QHBoxLayout(0));
    g_lay->addWidget(b_frame, lay_row, 0, 1, 2);
    lay_row++;

    initControlButtons(b_frame);


    connect(m_strategyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateComboPools()));
    connect(m_poolCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPoolChanged()));

}
void DefiStrategyPage::initControlButtons(QFrame *b_frame)
{
    QString path = AppCommonSettings::commonIconsPath();
    QString icon_file = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(QString("media-play.svg"));
    QToolButton *start_line_button = new  QToolButton(b_frame);
    start_line_button->setObjectName("start_line_button");
    start_line_button->setIcon(QIcon(icon_file));
    start_line_button->setToolTip("Start new line");
    b_frame->layout()->addWidget(start_line_button);
    start_line_button->setIconSize(QSize(64, 32));
    m_controls.insert("start_line_button", start_line_button);

    icon_file = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(QString("media-stop.svg"));
    QToolButton *stop_line_button = new  QToolButton(b_frame);
    stop_line_button->setObjectName("stop_line_button");
    stop_line_button->setIcon(QIcon(icon_file));
    stop_line_button->setToolTip("Stop current line");
    b_frame->layout()->addWidget(stop_line_button);
    stop_line_button->setIconSize(QSize(64, 32));
    m_controls.insert("stop_line_button", stop_line_button);

    icon_file = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(QString("document-save.svg"));
    QToolButton *close_step_button = new  QToolButton(b_frame);
    close_step_button->setObjectName("close_step_button");
    close_step_button->setIcon(QIcon(icon_file));
    b_frame->layout()->addWidget(close_step_button);
    close_step_button->setToolTip("Close current step");
    close_step_button->setIconSize(QSize(64, 32));
    m_controls.insert("close_step_button", close_step_button);

    icon_file = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(QString("exchange.png"));
    QToolButton *next_step_button = new  QToolButton(b_frame);
    next_step_button->setObjectName("next_step_button");
    next_step_button->setIcon(QIcon(icon_file));
    b_frame->layout()->addWidget(next_step_button);
    next_step_button->setToolTip("Open next step position");
    next_step_button->setIconSize(QSize(64, 32));
    m_controls.insert("next_step_button", next_step_button);

    connect(start_line_button, SIGNAL(clicked(bool)), this, SLOT(slotStartLine()));
    connect(stop_line_button, SIGNAL(clicked(bool)), this, SLOT(slotStopLine()));
    connect(close_step_button, SIGNAL(clicked(bool)), this, SLOT(slotCloseStep()));
    connect(next_step_button, SIGNAL(clicked(bool)), this, SLOT(slotNextStep()));
}
void DefiStrategyPage::slotUpdateComboPools()
{
    qDebug()<<QString("DefiStrategyPage::slotUpdateComboPools()  strategy count %1, cur strategy %2").arg(m_strategyCombo->count()).arg(curStrategy());
    m_poolCombo->clear();

    bool st_stable = curStrategyStable();
    int cid = defi_config.getChainID(curChainName());
    foreach (const DefiPoolV3 &p, defi_config.pools)
    {
        if (p.chain_id != cid) continue;
        if (p.is_stable && !st_stable) continue;
        if (!p.is_stable && st_stable) continue;

        QString p_desc = defi_config.shortPoolDescByAddr(p.address);
        m_poolCombo->addItem(p_desc, p.address);
    }
}
void DefiStrategyPage::slotPoolChanged()
{
    if (!m_dataObj) return;

    QString pool_addr = curPool();
    qDebug()<<QString("DefiStrategyPage::slotPoolChanged(%0)  POOL[%1]  STG[%2]").arg(curChainName()).arg(pool_addr).arg(curStrategy());
    if (pool_addr.isEmpty()) return;

    int l_index = m_dataObj->lineIndexOf(curStrategy(), pool_addr);
    qDebug()<<QString("DefiStrategyPage::slotPoolChanged()  l_index[%1]").arg(l_index);
    updateControlButtonsState(l_index);
}
QString DefiStrategyPage::curPool() const
{
    return m_poolCombo->currentData().toString().trimmed();
}
int DefiStrategyPage::curStrategy() const
{
    return m_strategyCombo->currentData().toInt();
}
bool DefiStrategyPage::curStrategyStable() const
{
    return (curStrategy() == dstStable);
}


// button slots
void DefiStrategyPage::slotStartLine()
{
    qDebug("--------DefiStrategyPage::slotStartLine()---------");
    StrategyLineData line;
    line.ts_open = DefiStrategyData::curTsPoint();
    line.strategy_type = curStrategy();
    line.pool_addr = curPool();

    //check settings
    QString err;
    line.start_parameters.liq_size = liqSize();
    if (line.start_parameters.liq_size < 0)
    {
        QString err = "Invalid start_parameters.liq_size value";
        qWarning()<<QString("DefiStrategyPage::slotStartLine() WARNING [%1]").arg(err);
        emit signalError(err);
        return;
    }
    line.start_parameters.range_width = rangeWidth();
    if (line.start_parameters.range_width < 0)
    {
        QString err = "Invalid start_parameters.range_width value";
        qWarning()<<QString("DefiStrategyPage::slotStartLine() WARNING [%1]").arg(err);
        emit signalError(err);
        return;
    }
    line.start_parameters.prior_asset_size = priorTokenPart();


    emit signalMsg("Line started OK!");
    m_dataObj->startLine(line);
    slotPoolChanged();
}
void DefiStrategyPage::slotStopLine()
{
    qDebug("--------DefiStrategyPage::slotStopLine()---------");
    controlButtonsDisable();
    m_startTimeEdit->setStyleSheet(QString("background-color: %1").arg(DT_LINE_DISABLED_COLOR));

    QString pool_addr = curPool();
    qDebug()<<QString("DefiStrategyPage::slotStopLine(%0)  POOL[%1]  STG[%2]").arg(curChainName()).arg(pool_addr).arg(curStrategy());
    int l_index = m_dataObj->lineIndexOf(curStrategy(), pool_addr);
    m_dataObj->finishLine(l_index);
}
void DefiStrategyPage::slotCloseStep()
{
    qDebug("--------DefiStrategyPage::slotCloseStep()---------");
    QString pool_addr = curPool();
    int l_index = m_dataObj->lineIndexOf(curStrategy(), pool_addr);

    m_dataObj->closeLastStep(l_index);
    updateControlButtonsState(l_index);

    qDebug("done!");
}
void DefiStrategyPage::slotNextStep()
{
    qDebug("--------DefiStrategyPage::slotNextStep()---------");
    QString pool_addr = curPool();
    int l_index = m_dataObj->lineIndexOf(curStrategy(), pool_addr);

    m_dataObj->openNextStep(l_index);
    updateControlButtonsState(l_index);
    qDebug("done!");
}



//private funcs
float DefiStrategyPage::liqSize() const
{
    const QLineEdit *edit = qobject_cast<const QLineEdit*>(m_controls.value(FULL_LIQ_SIZE_KEY));
    if (edit)
    {
        bool ok = false;
        float a = edit->text().trimmed().toFloat(&ok);
        if (!ok) qDebug("!ok");
        qDebug()<<edit->text().trimmed();
        if (ok && a > 0) return a;
    }
    else qDebug("liqSize QLineEdit is null");
    return -1;
}
float DefiStrategyPage::rangeWidth() const
{
    const QLineEdit *edit = qobject_cast<const QLineEdit*>(m_controls.value(RANGE_WIDTH_KEY));
    if (edit)
    {
        bool ok = false;
        float a = edit->text().trimmed().toFloat(&ok);
        if (ok && a > 0) return a;
    }
    else qDebug("rangeWidth QLineEdit is null");
    return -1;
}
quint16 DefiStrategyPage::priorTokenPart() const
{
    const QComboBox *combo = qobject_cast<const QComboBox*>(m_controls.value(PRIOR_TOKEN_PART_KEY));
    if (combo)
    {
        QString s = combo->currentText().trimmed();
        s.remove("%");
        bool ok = false;
        quint16 a = s.trimmed().toUInt(&ok);
        if (ok && a > 0) return a;
    }
    else qDebug("priorTokenPart QComboBox is null");
    return 50;
}
QToolButton* DefiStrategyPage::startLineBtn() const
{
    return (qobject_cast<QToolButton*>(m_controls.value("start_line_button")));
}
QToolButton* DefiStrategyPage::stopLineBtn() const
{
    return (qobject_cast<QToolButton*>(m_controls.value("stop_line_button")));
}
QToolButton* DefiStrategyPage::closeStepBtn() const
{
    return (qobject_cast<QToolButton*>(m_controls.value("close_step_button")));
}
QToolButton* DefiStrategyPage::nextStepBtn() const
{
    return (qobject_cast<QToolButton*>(m_controls.value("next_step_button")));
}


