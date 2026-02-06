#include "strategydata.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "lfile.h"
#include "lstaticxml.h"

#include <QDir>
#include <QDebug>
#include <QDomNode>
#include <QDateTime>


//DefiStrategyData
DefiStrategyData::DefiStrategyData(QString chainName, QObject *parent)
    :LSimpleObject(parent),
      m_chain(chainName.trimmed().toLower())
{
    setObjectName("defi_strategy_data_obj");

}
void DefiStrategyData::loadStrategyFile(bool &ok)
{
    qDebug("DefiStrategyData::loadStrategyFile");
    ok = false;
    m_lines.clear();
    if (m_chain.isEmpty()) {emit signalError(QString("DefiStrategyData: chain is empty")); return;}

    QString fname = AppCommonSettings::strategyDataFile().arg(m_chain);
    fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);
    if (!LFile::fileExists(fname))
    {
        qWarning()<<QString("DefiStrategyData: WARNING file not found [%1]").arg(fname);
        emit signalError(QString("DefiStrategyData: file not found [%1]").arg(fname));
        ok = true;
        return;
    }

    QDomNode root_node;
    QString err = LStaticXML::getDomRootNode(fname, root_node);
    if (!err.isEmpty())
    {
        qWarning()<<QString("DefiStrategyData: WARNING [%1]").arg(err);
        emit signalError(QString("DefiStrategyData: [%1]").arg(err));
        return;
    }

    qDebug()<<QString("Loaded strategy_data/%1, from[%2]").arg(m_chain).arg(fname);
    emit signalMsg(QString("Loaded strategy_data/%1, from[%2]").arg(m_chain).arg(fname));
    ok = true;

    //load line state
    QDomNode line_node = root_node.firstChild();
    while (!line_node.isNull())
    {
        if (line_node.nodeName() == "line")
            loadLineNode(line_node);

        line_node = line_node.nextSibling();
    }
}
void DefiStrategyData::rewriteStrategyFile()
{
    qDebug("DefiStrategyData::rewriteStrategyFile()");
    if (m_chain.isEmpty()) {emit signalError(QString("DefiStrategyData: chain is empty")); return;}

    QString fname = AppCommonSettings::strategyDataFile().arg(m_chain);
    fname = QString("%1%2%3").arg(AppCommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);

    QDomDocument dom;
    LStaticXML::createDomHeader(dom);
    QDomNode root_node = dom.createElement("data");
    dom.appendChild(root_node);

    foreach (const StrategyLineData &v, m_lines)
    {
        QDomElement line_node = dom.createElement("line");
        LStaticXML::setAttrNode(line_node, "chain", m_chain);
        v.fillLineNode(line_node);
        root_node.appendChild(line_node);
    }

    // write dom to xml file
    QString fdata = LStaticXML::domToFileData(dom);
    QString err = LFile::writeFile(fname, fdata);
    if (!err.isEmpty())
    {
        qWarning()<<QString("DefiStrategyData::saveStrategyFile() WARNING [%1]").arg(err);
        emit signalError(err);
    }
}
void DefiStrategyData::loadLineNode(const QDomNode &node)
{
    if (LStaticXML::getStringAttrValue("chain", node) != m_chain) return;

    qDebug()<<QString();
    qDebug("---------------DefiStrategyData::loadLineNode for chain----------------");
    bool ok;
    StrategyLineData line;
    const QDomNode params_node = node.namedItem("settings");
    line.loadLineNode(node, ok);
    if (!ok) {qWarning("DefiStrategyData::loadLineNode WARNING - can't load line_node"); return;}
    line.loadLineParameters(params_node, ok);
    if (!ok) {qWarning("DefiStrategyData::loadLineNode WARNING - can't load line_params"); return;}
    line.loadLineSteps(node, ok);
    if (!ok) {qWarning("DefiStrategyData::loadLineNode WARNING - can't load line_steps"); return;}


    m_lines.append(line);
    qDebug()<<m_lines.last().toStr();
    qDebug("LINE LOADED OK!!");
}
int DefiStrategyData::lineIndexOf(int s_type, const QString &pool_addr) const
{
    qDebug()<<QString("DefiStrategyData::lineIndexOf s_type[%1]  pool_addr[%2]").arg(s_type).arg(pool_addr);
    if (linesEmpty()) return -1;

    int n = lineCount();
    for (int i=0; i<n; i++)
    {
  //      qDebug()<<QString("i=%1").arg(i);
        const StrategyLineData &v = m_lines.at(i);
//        qDebug()<<v.toStr();

        if ((v.pool_addr == pool_addr) && (v.strategy_type == s_type)) return i;
    }
    return -1;
}
const StrategyLineData* DefiStrategyData::lineAt(int l_index) const
{
    if (l_index < 0 || l_index >= lineCount()) return NULL;
    return &m_lines.at(l_index);
}
bool DefiStrategyData::hasOpenedLine(int s_type, const QString &pool_addr) const
{
    if (m_lines.isEmpty()) return false;
    return (lineIndexOf(s_type, pool_addr) >= 0);
}
quint32 DefiStrategyData::curTsPoint()
{
    QDateTime ts = QDateTime::currentDateTime();
    return quint32(ts.currentSecsSinceEpoch());
}
QString DefiStrategyData::fromTsPointToStr(quint32 ts)
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(qint64(ts));
    return dt.toString("dd.MM.yyyy / hh:mm");
}
void DefiStrategyData::startLine(const StrategyLineData &line)
{
    m_lines.append(line);
    rewriteStrategyFile();
}
void DefiStrategyData::finishLine(int l_index)
{
    if (l_index < 0 || l_index >= lineCount()) {emit signalError(QString("invalid line_index: %1").arg(l_index)); return;}

    m_lines.removeAt(l_index);
    rewriteStrategyFile();
}
void DefiStrategyData::closeLastStep(int l_index)
{
    if (l_index < 0 || l_index >= lineCount()) {emit signalError(QString("invalid line_index: %1").arg(l_index)); return;}

    m_lines[l_index].closeLastStep();
    rewriteStrategyFile();
}
void DefiStrategyData::openNextStep(int l_index)
{
    if (l_index < 0 || l_index >= lineCount()) {emit signalError(QString("invalid line_index: %1").arg(l_index)); return;}

    m_lines[l_index].openNextStep();
    rewriteStrategyFile();
}



/////////////////StrategyLineData///////////////////////////

void StrategyLineData::fillLineNode(QDomElement &line_node) const
{
    if (line_node.isNull()) return;

    LStaticXML::setAttrNode(line_node, "pool", pool_addr, "strategy", QString::number(strategy_type));
    LStaticXML::setAttrNode(line_node, "ts_open", QString::number(ts_open));

    QDomDocument dom;
    QDomElement settings_node = dom.createElement("settings");
    LStaticXML::setAttrNode(settings_node, "liq_size", QString::number(start_parameters.liq_size));
    LStaticXML::setAttrNode(settings_node, "range_width", QString::number(start_parameters.range_width));
    LStaticXML::setAttrNode(settings_node, "prior_token_part", QString::number(start_parameters.prior_asset_size)); // %
    line_node.appendChild(settings_node);

    fillLineStepsNodes(line_node);
}
void StrategyLineData::fillLineStepsNodes(QDomElement &line_node) const
{
    if (steps.isEmpty()) return;

    QDomDocument dom;
    foreach (const StrategyLineStepState &v, steps)
    {
        QDomElement step_node = dom.createElement("step");
        LStaticXML::setAttrNode(step_node, "number", QString::number(v.number));
        v.fillStepNode(step_node);
        line_node.appendChild(step_node);
    }
}
void StrategyLineData::loadLineNode(const QDomNode &line_node, bool &ok)
{
    ok = false;
    if (line_node.isNull()) return;

    pool_addr = LStaticXML::getStringAttrValue("pool", line_node).trimmed().toLower();
    if (pool_addr.length() < 20) {qWarning("DefiStrategyData::loadLineNode WARNING - invalid pool_addr"); return;}

    // QString str_ts_open = LStaticXML::getStringAttrValue("ts_open", params_node.parentNode());
    // ts_open = str_ts_open.toUInt(&ok);
     //    if (!ok || ts_open == 0) {        qWarning()<<QString("DefiStrategyData::loadLineParameters WARNING - invalid value ts_open(%1), ok(%2)").arg(str_ts_open).arg(ok?"true":"false");        return;}

     ts_open = LStaticXML::getIntAttrValue("ts_open", line_node, 0);
     if (ts_open == 0) {qWarning("DefiStrategyData::loadLineNode WARNING - invalid value ts_open"); return;}

     strategy_type = LStaticXML::getIntAttrValue("strategy", line_node, 0);
     if ((strategy_type < dstFollowPrice) || (strategy_type > dstStable)) {qWarning("DefiStrategyData::loadLineNode WARNING - invalid value strategy_type"); return;}

     ok= true;
}
void StrategyLineData::loadLineParameters(const QDomNode &params_node, bool &ok)
{
    ok = false;
    if (params_node.isNull()) return;

    start_parameters.liq_size = LStaticXML::getDoubleAttrValue("liq_size", params_node);
    if (start_parameters.liq_size <= 0) {qWarning("DefiStrategyData::loadLineParameters WARNING - invalid value liq_size"); return;}

    start_parameters.range_width = LStaticXML::getDoubleAttrValue("range_width", params_node);
    if (start_parameters.range_width <= 0) {qWarning("DefiStrategyData::loadLineParameters WARNING - invalid value range_width"); return;}

    start_parameters.prior_asset_size = LStaticXML::getDoubleAttrValue("prior_token_part", params_node);
    if (start_parameters.prior_asset_size <= 0) {qWarning("DefiStrategyData::loadLineParameters WARNING - invalid value prior_token_part"); return;}

    ok= true;
}
void StrategyLineData::loadLineSteps(const QDomNode &node, bool &ok)
{
    ok = false;
    steps.clear();

    QDomNode step_node = node.firstChild();
    while (!step_node.isNull())
    {
        if (step_node.nodeName() == "step")
        {
            StrategyLineStepState l_step;
            l_step.loadStepNode(step_node, ok);
            if (!ok) return;

            l_step.setPriorIndex(pricePriorIndex(), amoutPriorIndex());
            steps.append(l_step);
        }
        step_node = step_node.nextSibling();
    }

    ok = true;
}
void StrategyLineData::reset()
{
    strategy_type = -1;
    ts_open = ts_close = 0;
    pool_addr.clear();
    start_parameters.reset();
}
bool StrategyLineData::lastStepOpened() const
{
    if (isFinished()) return false;
    if (steps.isEmpty())  return false;
    return steps.last().isOpened();
}
QString StrategyLineData::toStr() const
{
    QString s("StrategyLineData: ");
    s = QString("%1  pool[%2]  type=%3 steps[%4])").arg(s).arg(pool_addr).arg(strategy_type).arg(steps.count());
    return s ;
}
void StrategyLineData::closeLastStep()
{
    if (steps.isEmpty()) {qWarning("StrategyLineData::closeLastStep() WARNING steps.isEmpty()"); return;}

    int i = steps.count() - 1;
    steps[i].ts_close = DefiStrategyData::curTsPoint();
    steps[i].prices.exit_price = 199.99;
    steps[i].amounts.rewards.first = 0.01;
    steps[i].amounts.rewards.second = 0.05;

}
void StrategyLineData::openNextStep()
{
    StrategyLineStepState next_step;
    next_step.ts_open = DefiStrategyData::curTsPoint();
    next_step.number = steps.count() + 1;

    // set TEST values
    next_step.prices.start_price = 99.99;
    next_step.prices.p_range.first = 155;
    next_step.prices.p_range.second = 175;
    next_step.prices.t_range.first = -288654;
    next_step.prices.t_range.second = -262654;

    next_step.amounts.deposited.first = 1.0;
    next_step.amounts.deposited.second = 2.0;

    next_step.setPriorIndex(pricePriorIndex(), amoutPriorIndex());
    steps.append(next_step);
}
quint8 StrategyLineData::pricePriorIndex() const
{
    int i = defi_config.getPriorPriceIndexByPoolAddr(pool_addr);
    return ((i == 1) ? 1 : 0);
}
quint8 StrategyLineData::amoutPriorIndex() const
{
    int i = defi_config.getPriorAmountIndexByPoolAddr(pool_addr);
    return ((i == 1) ? 1 : 0);
}
QStringList StrategyLineData::tableStepRowData(int step_number) const
{
    if (steps.isEmpty()) return QStringList();

    foreach (const StrategyLineStepState &v, steps)
        if (v.number == step_number) return v.tableStepRowData();

    return QStringList();
}
void StrategyLineData::getCurrentLiqSize(QPair<float, float> &liq) const
{
    liq.first = liq.second = 0;
    quint8 p_index = amoutPriorIndex();
    if (steps.isEmpty())
    {
        if (p_index == 0) liq.first = start_parameters.liq_size;
        else liq.second = start_parameters.liq_size;
        return;
    }

    if (steps.last().isOpened()) return;

    const StrategyLineStepAmounts &a = steps.last().amounts;
    liq.first = a.closed.first + a.rewards.first;
    liq.second = a.closed.second + a.rewards.second;
}

/////////////////StrategyLineParameters///////////////////////////
void StrategyLineParameters::reset()
{
    liq_size = range_width = -1;
    prior_asset_size = 0;
}


/////////////////StrategyLineStepState///////////////////////////
void StrategyLineStepState::reset()
{
    number = 0;
    ts_open = ts_close = pid = 0;
    prices.reset();
    amounts.reset();
}
void StrategyLineStepState::fillStepNode(QDomElement &step_node) const
{
    LStaticXML::setAttrNode(step_node, "ts_open", QString::number(ts_open), "ts_close", QString::number(ts_close));

    QDomDocument dom;
    //fill prices block
    QDomElement price_node = dom.createElement("price");
    LStaticXML::setAttrNode(price_node, "start", QString::number(prices.start_price), "exit", QString::number(prices.exit_price));
    step_node.appendChild(price_node);
    QDomElement range_node = dom.createElement("range");
    LStaticXML::setAttrNode(range_node, "p1", QString::number(prices.p_range.first), "p2", QString::number(prices.p_range.second));
    LStaticXML::setAttrNode(range_node, "tick1", QString::number(prices.t_range.first), "tick2", QString::number(prices.t_range.second));
    step_node.appendChild(range_node);

    //fill amounts block
    QDomElement amounts_node = dom.createElement("amounts");
    step_node.appendChild(amounts_node);

    QDomElement deposited_node = dom.createElement("deposited");
    LStaticXML::setAttrNode(deposited_node, "asset0", QString::number(amounts.deposited.first), "asset1", QString::number(amounts.deposited.second));
    amounts_node.appendChild(deposited_node);
    QDomElement closed_node = dom.createElement("closed");
    LStaticXML::setAttrNode(closed_node, "asset0", QString::number(amounts.closed.first), "asset1", QString::number(amounts.closed.second));
    amounts_node.appendChild(closed_node);
    QDomElement rewards_node = dom.createElement("rewards");
    LStaticXML::setAttrNode(rewards_node, "asset0", QString::number(amounts.rewards.first), "asset1", QString::number(amounts.rewards.second));
    amounts_node.appendChild(rewards_node);

    /*
        <step number="1" ts_open="234234234" ts_close="0" >
            <price start="178.9" exit="-1" />
            <range p1="130.65" p2="230.16" tick1="-283748" tick2="-263781" />
            <amounts>
                <deposited asset0="0.51" asset1="165.45" />
                <closed asset0="0.0" asset1="265.45" />
                <rewards asset0="0.0056" asset1="5.45" />
            </amounts>
        </step>
    */
}
QStringList StrategyLineStepState::tableStepRowData() const
{
//    headers << "Step" << "Open time" << "Exit time" << "Price range" << "Deposited" <<
  //             "Current assets" << "Rewards" << "Step result";

    QStringList row_data;
    row_data << QString::number(number) << DefiStrategyData::fromTsPointToStr(ts_open);
    if (ts_close > 0) row_data << DefiStrategyData::fromTsPointToStr(ts_close);
    else row_data << "---";
    row_data << prices.strPriceRange();
    row_data << amounts.strAssetsSum("nested");
    row_data << amounts.strAssetsSum("assets");
    row_data << amounts.strAssetsSum("reward");
    row_data << strResult();

    return row_data;
}
void StrategyLineStepState::loadStepNode(const QDomNode &node, bool &ok)
{
    ok = false;
    if (node.isNull()) return;

    number = LStaticXML::getIntAttrValue("number", node, 0);
    if (number == 0) {qWarning("StrategyLineStepState WARNING: invalid number attr"); return;}
    ts_open = LStaticXML::getIntAttrValue("ts_open", node, 0);
    if (ts_open < 1000000) {qWarning("StrategyLineStepState WARNING: invalid ts_open attr"); return;}
    ts_close = LStaticXML::getIntAttrValue("ts_close", node, 0);

    //load prices block
    const QDomNode price_node = node.namedItem("price");
    if (price_node.isNull()) {qWarning("StrategyLineStepState WARNING: price_node is NULL"); return;}
    prices.start_price = LStaticXML::getDoubleAttrValue("start", price_node);
    if (prices.start_price <= 0) {qWarning("StrategyLineStepState WARNING: invalid start_price attr"); return;}
    prices.exit_price = LStaticXML::getDoubleAttrValue("exit", price_node);

    const QDomNode range_node = node.namedItem("range");
    if (range_node.isNull()) {qWarning("StrategyLineStepState WARNING: range_node is NULL"); return;}
    prices.p_range.first = LStaticXML::getDoubleAttrValue("p1", range_node, -1);
    if (prices.p_range.first <= 0) {qWarning("StrategyLineStepState WARNING: invalid p1 attr"); return;}
    prices.p_range.second = LStaticXML::getDoubleAttrValue("p2", range_node, -1);
    if (prices.p_range.second <= 0 || prices.p_range.first >= prices.p_range.second) {qWarning("StrategyLineStepState WARNING: invalid p2 attr"); return;}
    prices.t_range.first = LStaticXML::getIntAttrValue("tick1", range_node, -999);
    if (prices.t_range.first == 0) {qWarning("StrategyLineStepState WARNING: invalid tick1 attr"); return;}
    prices.t_range.second = LStaticXML::getIntAttrValue("tick2", range_node, -999);
    if (prices.t_range.second == 0 || prices.t_range.first >= prices.t_range.second) {qWarning("StrategyLineStepState WARNING: invalid tick2 attr"); return;}

    //load amounts block
    const QDomNode amount_node = node.namedItem("amounts");
    if (amount_node.isNull()) {qWarning("StrategyLineStepState WARNING: amounts_node is NULL"); return;}
    const QDomNode deposited_node = amount_node.namedItem("deposited");
    if (deposited_node.isNull()) {qWarning("StrategyLineStepState WARNING: amounts deposited_node is NULL"); return;}
    const QDomNode closed_node = amount_node.namedItem("closed");
    if (closed_node.isNull()) {qWarning("StrategyLineStepState WARNING: amounts closed_node is NULL"); return;}
    const QDomNode rewards_node = amount_node.namedItem("rewards");
    if (rewards_node.isNull()) {qWarning("StrategyLineStepState WARNING: amounts rewards_node is NULL"); return;}

    //deposited
    amounts.deposited.first = LStaticXML::getDoubleAttrValue("asset0", deposited_node, -1);
    if (amounts.deposited.first < 0) {qWarning("StrategyLineStepState WARNING: invalid deposited_amounts.asset0 attr"); return;}
    amounts.deposited.second = LStaticXML::getDoubleAttrValue("asset1", deposited_node, -1);
    if (amounts.deposited.second < 0) {qWarning("StrategyLineStepState WARNING: invalid deposited_amounts.asset1 attr"); return;}
    //closed
    amounts.closed.first = LStaticXML::getDoubleAttrValue("asset0", closed_node, -1);
    if (amounts.closed.first < 0) {qWarning("StrategyLineStepState WARNING: invalid closed_amounts.asset0 attr"); return;}
    amounts.closed.second = LStaticXML::getDoubleAttrValue("asset1", closed_node, -1);
    if (amounts.closed.second < 0) {qWarning("StrategyLineStepState WARNING: invalid closed_amounts.asset1 attr"); return;}
    //rewards
    amounts.rewards.first = LStaticXML::getDoubleAttrValue("asset0", rewards_node, -1);
    if (amounts.rewards.first < 0) {qWarning("StrategyLineStepState WARNING: invalid rewards.asset0 attr"); return;}
    amounts.rewards.second = LStaticXML::getDoubleAttrValue("asset1", rewards_node, -1);
    if (amounts.rewards.second < 0) {qWarning("StrategyLineStepState WARNING: invalid rewards.asset1 attr"); return;}


    ok= true;
}
void StrategyLineStepState::setPriorIndex(quint8 i_p, quint8 i_a)
{
    //prices.prior_index = 0;
    amounts.prior_index = 0;
    //if (i_p == 1) prices.prior_index = 1;
    if (i_a == 1) amounts.prior_index = 1;
}
QString StrategyLineStepState::strResult() const
{
    float res = amounts.totalStepResult();
    if (res < -100) return "---";

    QString s_res = QString::number(res, 'f', 1);
    if (res < 0) return QString("%1 %").arg(s_res);
    else if (res > 0.5) return QString("+%1 %").arg(s_res);

    return QString("%1 %").arg(s_res);
}

/////////////////StrategyLineStepAmounts & StrategyLineStepPrices ///////////////////////////
void StrategyLineStepAmounts::reset()
{
    prior_index = 0;
    deposited.first = deposited.second = 0;
    closed.first = closed.second = 0;
    rewards.first = rewards.second = 0;
}
void StrategyLineStepPrices::reset()
{
    //prior_index = 0;
    start_price = exit_price = -1;
    p_range.first = p_range.second = -1;
    t_range.first = t_range.second = 0;
}
QString StrategyLineStepPrices::strPriceRange() const
{
    float p1 = p_range.first;
    float p2 = p_range.second;
    quint8 prec = AppCommonSettings::interfacePricePrecision(p1);
    return QString("[%1 : %2]").arg(QString::number(p1, 'f', prec)).arg(QString::number(p2, 'f', prec));
}
QString StrategyLineStepAmounts::strAssetsSum(QString amount_type) const
{
    float a0 = 0;
    float a1 = 0;

    if (amount_type == "nested") {a0 = deposited.first; a1 = deposited.second;}
    else if (amount_type == "reward") {a0 = rewards.first; a1 = rewards.second;}
    else if (amount_type == "assets") {a0 = closed.first; a1 = closed.second;}
    else return "??";

    //quint8 prec0 = AppCommonSettings::interfacePricePrecision(a0);
    //quint8 prec1 = AppCommonSettings::interfacePricePrecision(a1);
    //return QString("%1 / %2").arg(QString::number(a0, 'f', prec0)).arg(QString::number(a1, 'f', prec1));

    return QString("%1 / %2").arg(QString::number(a0)).arg(QString::number(a1));
}
float StrategyLineStepAmounts::totalStepResult() const
{

    return -101;
}
float StrategyLineStepAmounts::userTokenSum(const QPair<float, float> &pair, float p_user) const
{
    float size0 = pair.first;
    float size1 = pair.second;

    float amount = -1;
    if (prior_index == 0)
    {
        amount = size0;
        if (size1 > 0) amount += (size1*p_user);
    }
    else
    {
        amount = size1;
        if (size0 > 0) amount += (size0*p_user);
    }
    return amount;
}



