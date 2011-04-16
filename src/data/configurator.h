/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: configurator.h 632 2010-02-28 16:38:28Z gotthardp $
 */

#ifndef _CONFIGURATOR_H
#define _CONFIGURATOR_H

class ConfigProvider
{
public:
  virtual ~ConfigProvider() {}

  virtual long get_config_long(const std::wstring& section, const std::wstring& parameter, long def = 0) const = 0;
  virtual float get_config_float(const std::wstring& section, const std::wstring& parameter, float def = 0.0f) const = 0;
};

class ConfigReader
{
public:
  ConfigReader() : m_config_provider(NULL) {}
  ConfigReader(ConfigProvider* config_provider) : m_config_provider(config_provider) {}
  virtual ~ConfigReader() {}

  void set_config_provider(ConfigProvider* config_provider)
  { m_config_provider = config_provider; }
  ConfigProvider* get_config_provider() const
  { return m_config_provider; }

  long get_config_long(const std::wstring& section,
    const std::wstring& parameter, long def = 0) const
  { return m_config_provider ? m_config_provider->get_config_long(section, parameter, def) : def; }
  float get_config_float(const std::wstring& section,
    const std::wstring& parameter, float def = 0.0f) const
  { return m_config_provider ? m_config_provider->get_config_float(section, parameter, def) : def; }

private:
  ConfigProvider* m_config_provider;
};

#endif /* _CONFIGURATOR_H */

// $Id: configurator.h 632 2010-02-28 16:38:28Z gotthardp $
