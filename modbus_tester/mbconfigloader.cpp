#include "mbconfigloader.h"
#include "mbdeviceemulator.h"
#include "lstaticxml.h"


#include <QDomNode>
#include <QFile>


/////////////// MBConfigLoader ///////////////////
MBConfigLoader::MBConfigLoader(QObject *parent)
    :LSimpleObject(parent),
    m_configName(QString())
{
    setObjectName("mb_config_loader");

}
void MBConfigLoader::tryLoadConfig(bool &ok)
{
    ok = false;
    QString msg = QString("MBConfigLoader: try load config file [%1] ..........").arg(m_configName);
    emit signalMsg(msg);

    MBDeviceEmulator *emulator = qobject_cast<MBDeviceEmulator*>(parent());
    if (!emulator)
    {
        msg = QString("MBConfigLoader: parent object is not device_emulator.");
        emit signalError(msg);
        return;
    }

    if (m_configName.trimmed().isEmpty())
    {
        msg = QString("MBConfigLoader: config filename is empty.").arg(m_configName);
        emit signalError(msg);
        return;
    }

    QFile f_xml(m_configName);
    if (!f_xml.exists(m_configName))
    {
        msg = QString("MBConfigLoader: config file [%1] not found.").arg(m_configName);
        emit signalError(msg);
        return;
    }

    QDomDocument dom;
    if (!dom.setContent(&f_xml))
    {
        msg = QString("MBConfigLoader: invalid load file to XML document [%1].").arg(m_configName);
        emit signalError(msg);
        return;
    }

    emulator->reset();
    loadConfig(dom);
    ok = true;
}
void MBConfigLoader::loadConfig(const QDomDocument &dom)
{
    QString msg;
    QString root_node_name("config");
    QDomNode root_node = dom.namedItem(root_node_name);
    if (root_node.isNull())
    {
        msg = QString("MBConfigLoader: invalid struct XML document [%1], node <%2> not found.").arg(m_configName).arg(root_node_name);
        emit signalError(msg);
        return;
    }

    QDomNode child_node = root_node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == QString("device"))
        {
            loadDevice(child_node);
        }
        child_node = child_node.nextSibling();
    }

    msg = QString("MBConfigLoader: config loaded ok!");
    emit signalMsg(msg);
}
void MBConfigLoader::loadDevice(const QDomNode &node)
{
    QString msg;
    MBDeviceEmulator *emulator = qobject_cast<MBDeviceEmulator*>(parent());
    int addr = LStaticXML::getIntAttrValue("addr", node);
    if (addr < 0)
    {
        msg = QString("MBConfigLoader::loadDevice: invalid device address attr_node=[addr].");
        emit signalError(msg);
    }
    else
    {
        emulator->addDevice(quint8(addr));
        loadDeviceSignals(quint8(addr), node);
        msg = QString("     Device loaded: addr=%1,  signals count %2.").arg(addr).arg(emulator->deviceSignalsCount(quint8(addr)));
        emit signalMsg(msg);
    }
}
void MBConfigLoader::loadDeviceSignals(quint8 addr, const QDomNode &node)
{
    MBDeviceEmulator *emulator = qobject_cast<MBDeviceEmulator*>(parent());
    EmulValueSettings evs;
    evs.disableSettings();

    QDomNode child_node = node.firstChild();
    while (!child_node.isNull())
    {
        if (child_node.nodeName() == QString("emul_params"))
        {
            loadEmulParams(evs, child_node);
            emit signalMsg(QString("        loaded rack(%1) emul params - %2").arg(addr).arg(evs.toStr()));
        }
        else if (child_node.nodeName() == QString("signal"))
        {
            loadDeviceSignal(addr, child_node);
        }
        child_node = child_node.nextSibling();
    }
    emulator->setEmuValueSettings(addr, evs);
}
void MBConfigLoader::loadDeviceSignal(quint8 addr, const QDomNode &node)
{
    MBDeviceEmulator *emulator = qobject_cast<MBDeviceEmulator*>(parent());

    bool sig_ok = false;
    QString s_reg_pos = LStaticXML::getStringAttrValue("pos", node);
    int reg_pos = -1;
    if (!s_reg_pos.isEmpty())
    {
        bool ok;
        reg_pos = s_reg_pos.toUInt(&ok, 16);
        if (!ok)
        {
            reg_pos = s_reg_pos.toUInt(&ok, 10);
            if (!ok) reg_pos = -1;
        }
    }
    if (reg_pos < 0) qWarning("MBConfigLoader::loadDeviceSignals WARNING  invalid signal reg_pos attr");
    else
    {
        int reg_type = LStaticXML::getIntAttrValue("reg_type", node);
        if (reg_type != MBRegisterInfo::estBit_1 && reg_type != MBRegisterInfo::estBit_16) qWarning("MBConfigLoader::loadDeviceSignals WARNING  invalid signal reg_type attr");
        else
        {
            if (reg_type == MBRegisterInfo::estBit_1)
            {
                int bit_index = LStaticXML::getIntAttrValue("bit_index", node);
                if (bit_index < 0) qWarning("MBConfigLoader::loadDeviceSignals WARNING  invalid signal bit_index attr");
                else
                {
                    MBRegisterInfo info(reg_type, quint16(reg_pos), quint8(bit_index));
                    emulator->addDeviceSignal(addr, info);
                    sig_ok = true;
                }
            }
            else
            {
                MBRegisterInfo info(reg_type, quint16(reg_pos));
                emulator->addDeviceSignal(addr, info);
                sig_ok = true;
            }
        }
    }


    //try load emul settings
    if (! sig_ok) return;
    QDomNode emul_params_node = node.namedItem("emul_params");
    if (!emul_params_node.isNull())
    {
        EmulValueSettings evs;
        loadEmulParams(evs, emul_params_node);
        emulator->setEmuValueSettings(addr, evs, emulator->deviceSignalsCount(addr)-1);

        emit signalMsg(QString("        loaded signal emul params - %1").arg(evs.toStr()));
    }
}
void MBConfigLoader::loadEmulParams(EmulValueSettings &evs, const QDomNode &node)
{
    bool ok;
    double v = LStaticXML::getStringAttrValue("value", node).toDouble(&ok);
    if (ok) evs.base_value = v;
    else qWarning()<<QString("MBConfigLoader::loadEmulParams WARNING - invalid base_value %1").arg(LStaticXML::getStringAttrValue("value", node));

    v = LStaticXML::getStringAttrValue("err", node).toDouble(&ok);
    if (ok) evs.err = v;
    else qWarning()<<QString("MBConfigLoader::loadEmulParams WARNING - invalid error value %1").arg(LStaticXML::getStringAttrValue("err", node));

    v = LStaticXML::getStringAttrValue("factor", node).toDouble(&ok);
    if (ok) evs.factor = v;
    else qWarning()<<QString("MBConfigLoader::loadEmulParams WARNING - invalid factor value %1").arg(LStaticXML::getStringAttrValue("factor", node));

    v = LStaticXML::getStringAttrValue("adder", node).toDouble(&ok);
    if (ok) evs.adder = v;
    else qWarning()<<QString("MBConfigLoader::loadEmulParams WARNING - invalid adder value %1").arg(LStaticXML::getStringAttrValue("adder", node));
}




