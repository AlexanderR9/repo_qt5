#include "defimintdialog.h"
#include "txdialog.h"
#include "deficonfig.h"
#include "ltable.h"
#include "appcommonsettings.h"

#include <QDebug>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>


//DefiMintDialog
DefiMintDialog::DefiMintDialog(TxDialogData &data, QWidget *parent)
    :QDialog(parent),
      m_data(data),
      m_apply(false)
{
    setupUi(this);
    setObjectName(QString("mint_pos_dialog"));
    setWindowTitle(QString("Mint position (%1)").arg(m_data.chain_name.trimmed().toUpper()));
    resize(900, 500);
    setModal(true);


    init();
    initBaseTokens();

    connect(tokenComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBaseTokenChainged()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

}
void DefiMintDialog::init()
{
    poolComboBox->clear();
    tokenComboBox->clear();

    LTable::fullClearTable(poolStateTable);
    LTable::fullClearTable(previewMintTable);

    QStringList headers;
    headers << "State value";
    LTable::setTableHeaders(poolStateTable, headers, Qt::Horizontal);
    headers.clear();
    headers << "TVL, kUsdt" << "Tick" << "Price" << "Token0 balance" << "Token1 balance";
    LTable::setTableHeaders(poolStateTable, headers, Qt::Vertical);
    LTable::createAllItems(poolStateTable);
    poolStateTable->setSelectionMode(QAbstractItemView::NoSelection);
    LTable::resizeTableContents(poolStateTable);

    updatePoolStateButton->setIcon(QIcon(QString("%1/view-refresh.svg").arg(AppCommonSettings::commonIconsPath())));
    updatePoolStateButton->setIconSize(QSize(22, 22));

}
void DefiMintDialog::initBaseTokens()
{
    int cid = defi_config.getChainID(m_data.chain_name);
    int chain_index = defi_config.chainIndexOf(cid);
    QString chain_icon = defi_config.chains.at(chain_index).fullIconPath();

    m_tokens.clear();
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id == cid)
        {
            tokenComboBox->addItem(QIcon(chain_icon), v.name);
            m_tokens.insert(v.name, v.address);
        }
    }
    //tokenComboBox->setIconSize(QSize(12, 12));

    slotBaseTokenChainged();
}
void DefiMintDialog::slotBaseTokenChainged()
{
    int cid = defi_config.getChainID(m_data.chain_name);
    int chain_index = defi_config.chainIndexOf(cid);
    QString chain_icon = defi_config.chains.at(chain_index).fullIconPath();

    QString pool_info;
    poolComboBox->clear();
    QString t_addr = baseTokenAddrSelected();
    qDebug()<<QString("DefiMintDialog::slotBaseTokenChainged() token[%1]  address[%2]").arg(tokenComboBox->currentText()).arg(t_addr);
    foreach (const DefiPoolV3 &v, defi_config.pools)
    {
        if (v.chain_id != cid) continue;
        if (v.token0_addr == t_addr || v.token1_addr == t_addr)
        {
            QString t0 = defi_config.tokenNameByAddress(v.token0_addr, cid);
            QString t1 = defi_config.tokenNameByAddress(v.token1_addr, cid);
            pool_info = QString("%1/%2 (%3)").arg(t0).arg(t1).arg(v.strFloatFee());
            poolComboBox->addItem(QIcon(chain_icon), pool_info);
        }
    }

}
QString DefiMintDialog::baseTokenAddrSelected() const
{
    QString t_name = tokenComboBox->currentText().trimmed();
    if (t_name.isEmpty()) return QString();
    return m_tokens.value(t_name, QString());
}
void DefiMintDialog::slotApply()
{
    qDebug("DefiMintDialog::slotApply()");
    m_apply = false;
    //close();
}


