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

#ifndef XCSOAR_DATA_FIELD_BASE_HPP
#define XCSOAR_DATA_FIELD_BASE_HPP

#include "Compiler.h"

#include <tchar.h>

#define OUTBUFFERSIZE 128

class ComboList;

class DataField
{
public:
  enum Type {
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_REAL,
    TYPE_ENUM,
    TYPE_FILE,
  };

  enum DataAccessKind_t {
    daChange,
    daSpecial,
  };

  typedef void (*DataAccessCallback_t)(DataField * Sender, DataAccessKind_t Mode);

  bool SupportCombo;  // all Types dataField support combolist except DataFieldString.

protected:
  const Type type;

  DataAccessCallback_t mOnDataAccess;
  bool mItemHelp;

private:
  int mUsageCounter;
  bool mDisableSpeedup;
  bool mDetachGUI;

public:
  DataField(Type _type,
            DataAccessCallback_t OnDataAccess = NULL);
  virtual ~DataField(void) {}

  void SetDataAccessCallback(DataAccessCallback_t _data_access_callback) {
    mOnDataAccess = _data_access_callback;
  }

  Type GetType() const {
    return type;
  }

  void Special(void);
  virtual void Inc(void);
  virtual void Dec(void);

  gcc_pure
  virtual int GetAsInteger() const;

  gcc_pure
  virtual const TCHAR *GetAsString() const;

  gcc_pure
  virtual const TCHAR *GetAsDisplayString() const;

  virtual void SetAsInteger(int Value);
  virtual void SetAsString(const TCHAR *Value);

  void
  Use(void)
  {
    mUsageCounter++;
  }

  int
  Unuse(void)
  {
    mUsageCounter--;
    return mUsageCounter;
  }
  virtual void EnableItemHelp(bool value) {};

  // allows combolist to iterate all values
  void SetDisableSpeedUp(bool bDisable) { mDisableSpeedup = bDisable; }
  bool GetDisableSpeedUp(void) { return mDisableSpeedup; }
  // allows combolist to iterate all values w/out triggering external events
  void SetDetachGUI(bool bDetachGUI) { mDetachGUI = bDetachGUI; }
  bool GetDetachGUI(void) { return mDetachGUI; }

  gcc_malloc
  virtual ComboList *CreateComboList() const { return NULL; }

  virtual void
  SetFromCombo(int iDataFieldIndex, TCHAR *sValue)
  {
    SetAsInteger(iDataFieldIndex);
  }
  bool GetItemHelpEnabled() { return mItemHelp; }

  void CopyString(TCHAR * szStringOut, bool bFormatted);
};

#endif
