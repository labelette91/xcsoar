/*
  Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "DeviceEditWidget.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"
#include "Compiler.h"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"
#include "Compatibility/string.h"
#include "DataField/Enum.hpp"
#include "Device/Register.hpp"
#include "Device/Driver.hpp"

#ifdef _WIN32_WCE
#include "Device/Windows/Enumerator.hpp"
#endif

#ifdef ANDROID
#include "Android/BluetoothHelper.hpp"
#ifdef IOIOLIB
#include "Device/Port/AndroidIOIOUartPort.hpp"
#endif
#endif

enum ControlIndex {
  Port, BaudRate, BulkBaudRate, TCPPort, Driver,
};

static gcc_constexpr_data struct {
  DeviceConfig::PortType type;
  const TCHAR *label;
} port_types[] = {
  { DeviceConfig::PortType::DISABLED, N_("Disabled") },
#ifdef _WIN32_WCE
  { DeviceConfig::PortType::AUTO, N_("GPS Intermediate Driver") },
#endif
#ifdef ANDROID
  { DeviceConfig::PortType::INTERNAL, N_("Built-in GPS") },
#endif

  /* label not translated for now, until we have a TCP port
     selection UI */
  { DeviceConfig::PortType::TCP_LISTENER, _T("TCP port") },

  { DeviceConfig::PortType::SERIAL, NULL } /* sentinel */
};

/** the number of fixed port types (excludes Serial, Bluetooth and IOIOUart) */
static gcc_constexpr_data unsigned num_port_types = ARRAY_SIZE(port_types) - 1;

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static DeviceEditWidget *instance;

static unsigned
AddPort(DataFieldEnum &df, DeviceConfig::PortType type,
        const TCHAR *text, const TCHAR *display_string=NULL,
        const TCHAR *help=NULL)
{
  /* the uppper 16 bit is the port type, and the lower 16 bit is a
     serial number to make the enum id unique */

  unsigned id = ((unsigned)type << 16) + df.Count();
  df.AddChoice(id, text, display_string, help);
  return id;
}

#if defined(HAVE_POSIX)

#include <dirent.h>
#include <unistd.h>

static bool
DetectSerialPorts(DataFieldEnum &df)
{
  DIR *dir = opendir("/dev");
  if (dir == NULL)
    return false;

  unsigned sort_start = df.Count();

  bool found = false;
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    /* filter "/dev/tty*" */
    if (memcmp(ent->d_name, "tty", 3) == 0) {
      /* filter out "/dev/tty0", ... (valid integer after "tty") */
      char *endptr;
      strtoul(ent->d_name + 3, &endptr, 10);
      if (*endptr == 0)
        continue;
    } else if (memcmp(ent->d_name, "rfcomm", 6) != 0)
      continue;

    char path[64];
    snprintf(path, sizeof(path), "/dev/%s", ent->d_name);
    if (access(path, R_OK|W_OK) == 0 && access(path, X_OK) < 0) {
      AddPort(df, DeviceConfig::PortType::SERIAL, path);
      found = true;
    }
  }

  closedir(dir);

  if (found)
    df.Sort(sort_start);

  return found;
}

#endif

#ifdef GNAV

static bool
DetectSerialPorts(DataFieldEnum &df)
{
  AddPort(df, DeviceConfig::PortType::SERIAL, _T("COM1:"), _T("Vario (COM1)"));
  AddPort(df, DeviceConfig::PortType::SERIAL, _T("COM2:"), _T("Radio (COM2)"));
  AddPort(df, DeviceConfig::PortType::SERIAL, _T("COM3:"), _T("Internal (COM3)"));
  return true;
}

#elif defined(_WIN32_WCE)

static bool
DetectSerialPorts(DataFieldEnum &df)
{
  PortEnumerator enumerator;
  if (enumerator.Error())
    return false;

  unsigned sort_start = df.Count();

  bool found = false;
  while (enumerator.Next()) {
    AddPort(df, DeviceConfig::PortType::SERIAL, enumerator.GetName(),
            enumerator.GetDisplayName());
    found = true;
  }

  if (found)
    df.Sort(sort_start);

  return found;
}

#endif

#if defined(WIN32) && !defined(HAVE_POSIX)

static void
FillDefaultSerialPorts(DataFieldEnum &df)
{
  for (unsigned i = 1; i <= 10; ++i) {
    TCHAR buffer[64];
    _stprintf(buffer, _T("COM%u:"), i);
    AddPort(df, DeviceConfig::PortType::SERIAL, buffer);
  }
}

#endif

static void
FillPortTypes(DataFieldEnum &df, const DeviceConfig &config)
{
  for (unsigned i = 0; port_types[i].label != NULL; i++) {
    unsigned id = AddPort(df, port_types[i].type,
                          gettext(port_types[i].label));

    if (port_types[i].type == config.port_type)
      df.Set(id);
  }
}

static void
FillSerialPorts(DataFieldEnum &df, const DeviceConfig &config)
{
#if defined(HAVE_POSIX)
  DetectSerialPorts(df);
#elif defined(WIN32)
#ifdef _WIN32_WCE
  if (!DetectSerialPorts(df))
#endif
    FillDefaultSerialPorts(df);
#endif

  if (config.port_type == DeviceConfig::PortType::SERIAL) {
    if (!df.Exists(config.path))
        AddPort(df, config.port_type, config.path);

    df.SetAsString(config.path);
  }
}

static void
FillAndroidBluetoothPorts(DataFieldEnum &df, const DeviceConfig &config)
{
#ifdef ANDROID
  JNIEnv *env = Java::GetEnv();
  jobjectArray bonded = BluetoothHelper::list(env);
  if (bonded == NULL)
    return;

  jsize n = env->GetArrayLength(bonded) / 2;
  for (jsize i = 0; i < n; ++i) {
    jstring address = (jstring)env->GetObjectArrayElement(bonded, i * 2);
    if (address == NULL)
      continue;

    const char *address2 = env->GetStringUTFChars(address, NULL);
    if (address2 == NULL)
      continue;

    jstring name = (jstring)env->GetObjectArrayElement(bonded, i * 2 + 1);
    const char *name2 = name != NULL
      ? env->GetStringUTFChars(name, NULL)
      : NULL;

    AddPort(df, DeviceConfig::PortType::RFCOMM, address2, name2);

    env->ReleaseStringUTFChars(address, address2);
    if (name2 != NULL)
      env->ReleaseStringUTFChars(name, name2);
  }

  env->DeleteLocalRef(bonded);

  if (config.port_type == DeviceConfig::PortType::RFCOMM &&
      !config.bluetooth_mac.empty()) {
    if (!df.Exists(config.bluetooth_mac))
      AddPort(df, DeviceConfig::PortType::RFCOMM, config.bluetooth_mac);

    df.SetAsString(config.bluetooth_mac);
  }
#endif
}

static void
FillAndroidIOIOPorts(DataFieldEnum &df, const DeviceConfig &config)
{
#if defined(ANDROID) && defined(IOIOLIB)
  df.EnableItemHelp(true);

  TCHAR tempID[4];
  TCHAR tempName[15];
  for (unsigned i = 0; i < AndroidIOIOUartPort::getNumberUarts(); i++) {
    _sntprintf(tempID, sizeof(tempID), _T("%d"), i);
    _sntprintf(tempName, sizeof(tempName), _T("IOIO Uart %d"), i);
    AddPort(df, DeviceConfig::PortType::IOIOUART, tempID, tempName,
            AndroidIOIOUartPort::getPortHelp(i));
  }

  if (config.port_type == DeviceConfig::PortType::IOIOUART &&
      config.ioio_uart_id < AndroidIOIOUartPort::getNumberUarts()) {
    _sntprintf(tempID,  sizeof(tempID), _T("%d"), config.ioio_uart_id);
    df.SetAsString(tempID);
  }
#endif
}

static void
FillPorts(DataFieldEnum &df, const DeviceConfig &config)
{
  FillPortTypes(df, config);
  FillSerialPorts(df, config);
  FillAndroidBluetoothPorts(df, config);
  FillAndroidIOIOPorts(df, config);
}

static void
FillBaudRates(DataFieldEnum &dfe)
{
  dfe.addEnumText(_T("1200"), 1200);
  dfe.addEnumText(_T("2400"), 2400);
  dfe.addEnumText(_T("4800"), 4800);
  dfe.addEnumText(_T("9600"), 9600);
  dfe.addEnumText(_T("19200"), 19200);
  dfe.addEnumText(_T("38400"), 38400);
  dfe.addEnumText(_T("57600"), 57600);
  dfe.addEnumText(_T("115200"), 115200);
}

static void
FillTCPPorts(DataFieldEnum &dfe)
{
  dfe.addEnumText(_T("4353"), 4353);
  dfe.addEnumText(_T("10110"), 10110);
}

static void
SetPort(DataFieldEnum &df, const DeviceConfig &config)
{
  switch (config.port_type) {
  case DeviceConfig::PortType::DISABLED:
  case DeviceConfig::PortType::AUTO:
  case DeviceConfig::PortType::INTERNAL:
  case DeviceConfig::PortType::TCP_LISTENER:
    break;

  case DeviceConfig::PortType::SERIAL:
    if (!df.Exists(config.path))
      AddPort(df, config.port_type, config.path);
    df.SetAsString(config.path);
    return;

  case DeviceConfig::PortType::RFCOMM:
    if (!df.Exists(config.bluetooth_mac))
      AddPort(df, DeviceConfig::PortType::RFCOMM, config.bluetooth_mac);

    df.SetAsString(config.bluetooth_mac);
    return;

  case DeviceConfig::PortType::IOIOUART:
    StaticString<16> buffer;
    buffer.UnsafeFormat(_T("%d"), config.ioio_uart_id);
    df.SetAsString(buffer);
    return;
  }

  for (unsigned i = 0; port_types[i].label != NULL; i++) {
    if (port_types[i].type == config.port_type) {
      df.SetAsString(gettext(port_types[i].label));
      break;
    }
  }
}

DeviceEditWidget::DeviceEditWidget(const DeviceConfig &_config)
  :RowFormWidget(UIGlobals::GetDialogLook(), Layout::Scale(80)),
   config(_config) {}


void
DeviceEditWidget::SetConfig(const DeviceConfig &_config)
{
  config = _config;

  WndProperty &port_control = GetControl(Port);
  DataFieldEnum &port_df = *(DataFieldEnum *)port_control.GetDataField();
  SetPort(port_df, config);
  port_control.RefreshDisplay();

  WndProperty &baud_control = GetControl(BaudRate);
  DataFieldEnum &baud_df = *(DataFieldEnum *)baud_control.GetDataField();
  baud_df.Set(config.baud_rate);
  baud_control.RefreshDisplay();

  WndProperty &bulk_baud_control = GetControl(BulkBaudRate);
  DataFieldEnum &bulk_baud_df = *(DataFieldEnum *)
    bulk_baud_control.GetDataField();
  bulk_baud_df.Set(config.bulk_baud_rate);
  bulk_baud_control.RefreshDisplay();

  WndProperty &tcp_port_control = GetControl(TCPPort);
  DataFieldEnum &tcp_port_df = *(DataFieldEnum *)
    tcp_port_control.GetDataField();
  tcp_port_df.Set(config.tcp_port);
  tcp_port_control.RefreshDisplay();

  WndProperty &driver_control = GetControl(Driver);
  DataFieldEnum &driver_df = *(DataFieldEnum *)driver_control.GetDataField();
  driver_df.SetAsString(config.driver_name);
  driver_control.RefreshDisplay();
}

gcc_pure
static bool
SupportsBulkBaudRate(const DataField &df)
{
  const TCHAR *driver_name = df.GetAsString();
  if (driver_name == NULL)
    return false;

  const struct DeviceRegister *driver = FindDriverByName(driver_name);
  if (driver == NULL)
    return false;

  return driver->SupportsBulkBaudRate();
}

gcc_pure
static DeviceConfig::PortType
GetPortType(DataField &df)
{
  unsigned port = df.GetAsInteger();

  if (port < num_port_types)
    return port_types[port].type;

  return (DeviceConfig::PortType)(port >> 16);
}

void
DeviceEditWidget::UpdateVisibilities()
{
  const DeviceConfig::PortType type = GetPortType(GetDataField(Port));

  GetControl(BaudRate).set_visible(DeviceConfig::UsesSpeed(type));
  GetControl(BulkBaudRate).set_visible(DeviceConfig::UsesSpeed(type) &&
                                       DeviceConfig::UsesDriver(type) &&
                                       SupportsBulkBaudRate(GetDataField(Driver)));
  GetControl(TCPPort).set_visible(DeviceConfig::UsesTCPPort(type));
  GetControl(Driver).set_visible(DeviceConfig::UsesDriver(type));
}

static void
OnDataField(gcc_unused DataField *df, DataField::DataAccessKind_t mode)
{
  if (mode == DataField::daChange)
    instance->UpdateVisibilities();
}

void
DeviceEditWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  instance = this;

  RowFormWidget::Prepare(parent, rc);

  DataFieldEnum *port_df = new DataFieldEnum(OnDataField);
  port_df->SetDetachGUI(true);
  FillPorts(*port_df, config);
  Add(_("Port"), NULL, port_df);

  DataFieldEnum *baud_rate_df = new DataFieldEnum(NULL);
  FillBaudRates(*baud_rate_df);
  baud_rate_df->Set(config.baud_rate);
  Add(_("Baudrate"), NULL, baud_rate_df);

  DataFieldEnum *bulk_baud_rate_df = new DataFieldEnum(NULL);
  bulk_baud_rate_df->addEnumText(_T("Default"), 0u);
  FillBaudRates(*bulk_baud_rate_df);
  bulk_baud_rate_df->Set(config.bulk_baud_rate);
  Add(_("Bulk baud rate"),
      _("The baud rate used for bulk transfers, such as task declaration or flight download."),
      bulk_baud_rate_df);

  DataFieldEnum *tcp_port_df = new DataFieldEnum(NULL);
  FillTCPPorts(*tcp_port_df);
  tcp_port_df->Set(config.tcp_port);
  Add(_("TCP Port"), NULL, tcp_port_df);

  DataFieldEnum *driver_df = new DataFieldEnum(OnDataField);
  driver_df->SetDetachGUI(true);
  const TCHAR *driver_name;
  for (unsigned i = 0; (driver_name = GetDriverNameByIndex(i)) != NULL; i++)
    driver_df->addEnumText(driver_name, GetDriverDisplayNameByIndex(i));

  driver_df->Sort(1);
  driver_df->SetAsString(config.driver_name);

  Add(_("Driver"), NULL, driver_df);

  port_df->SetDetachGUI(false);
  driver_df->SetDetachGUI(false);

  UpdateVisibilities();
}

/**
 * @return true if the value has changed
 */
static bool
FinishPortField(DeviceConfig &config, const DataFieldEnum &df)
{
  unsigned value = df.GetAsInteger();

  /* decode the port type from the upper 16 bits of the id; we don't
     need the rest, because that's just some serial we don't care
     about */
  const DeviceConfig::PortType new_type =
    (DeviceConfig::PortType)(value >> 16);
  switch (new_type) {
  case DeviceConfig::PortType::DISABLED:
  case DeviceConfig::PortType::AUTO:
  case DeviceConfig::PortType::INTERNAL:
  case DeviceConfig::PortType::TCP_LISTENER:
    if (new_type == config.port_type)
      return false;

    config.port_type = new_type;
    return true;

  case DeviceConfig::PortType::SERIAL:
    /* Serial Port */
    if (new_type == config.port_type &&
        _tcscmp(config.path, df.GetAsString()) == 0)
      return false;

    config.port_type = new_type;
    config.path = df.GetAsString();
    return true;

  case DeviceConfig::PortType::RFCOMM:
    /* Bluetooth */
    if (new_type == config.port_type &&
        _tcscmp(config.bluetooth_mac, df.GetAsString()) == 0)
      return false;

    config.port_type = new_type;
    config.bluetooth_mac = df.GetAsString();
    return true;

  case DeviceConfig::PortType::IOIOUART:
    /* IOIO UART */
    if (new_type == config.port_type &&
        config.ioio_uart_id == (unsigned)_ttoi(df.GetAsString()))
      return false;

    config.port_type = new_type;
    config.ioio_uart_id = (unsigned)_ttoi(df.GetAsString());
    return true;
  }

  /* unreachable */
  assert(false);
  return false;
}

bool
DeviceEditWidget::Save(bool &_changed, bool &require_restart)
{
  bool changed = false;

  changed |= FinishPortField(config, (const DataFieldEnum &)GetDataField(Port));

  if (config.UsesSpeed()) {
    changed |= SaveValue(BaudRate, config.baud_rate);
    changed |= SaveValue(BulkBaudRate, config.bulk_baud_rate);
  }

  if (config.UsesTCPPort())
    changed |= SaveValue(TCPPort, config.tcp_port);


  if (config.UsesDriver())
    changed |= SaveValue(Driver, config.driver_name.buffer(),
                         config.driver_name.MAX_SIZE);

  _changed |= changed;
  return true;
}
