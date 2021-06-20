/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include "../../misc/DataWrapper.hpp"
#include "../../misc/enum.hpp"
#include "../../../utility/result.hpp"

#include <malloc.h>

#include <Arduino.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  __attribute__ ((unused))
  static inline unsigned long millis(void)
  {
    return ::millis();
  }
  __attribute__ ((unused))
  static inline unsigned long micros(void)
  {
    return ::micros();
  }
  __attribute__ ((unused))
  static inline void delay(unsigned long milliseconds)
  {
    ::delay(milliseconds);
  }
  __attribute__ ((unused))
  static void delayMicroseconds(unsigned int us)
  {
    ::delayMicroseconds(us);
  }

  static inline void* heap_alloc(      size_t length) { return malloc(length); }
  static inline void* heap_alloc_psram(size_t length) { return malloc(length); }
  static inline void* heap_alloc_dma(  size_t length) { return malloc(length); } // aligned_alloc(16, length);
  static inline void heap_free(void* buf) { free(buf); }

  static inline void gpio_hi(std::uint32_t pin) { digitalWrite(pin, HIGH); }
  static inline void gpio_lo(std::uint32_t pin) { digitalWrite(pin, LOW); }
  static inline bool gpio_in(std::uint32_t pin) { return digitalRead(pin); }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void pinMode(std::int_fast16_t pin, pin_mode_t mode);
  inline void lgfxPinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    pinMode(pin, mode);
  }

//----------------------------------------------------------------------------
  struct FileWrapper : public DataWrapper
  {
    FileWrapper() : DataWrapper() { need_transaction = true; }

#if defined (ARDUINO) && defined (__SEEED_FS__)

    fs::File _file;
    fs::File *_fp;

    fs::FS *_fs = nullptr;
    void setFS(fs::FS& fs) {
      _fs = &fs;
      need_transaction = false;
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fp(nullptr) { setFS(fs); }
    FileWrapper(fs::FS& fs, fs::File* fp) : DataWrapper(), _fp(fp) { setFS(fs); }

    bool open(fs::FS& fs, const char* path) {
      setFS(fs);
      return open(path);
    }

    bool open(const char* path) override {
      fs::File file = _fs->open(path, "r");
      // この邪悪なmemcpyは、Seeed_FSのFile実装が所有権moveを提供してくれないのにデストラクタでcloseを呼ぶ実装になっているため、
      // 正攻法ではFileをクラスメンバに保持できない状況を打開すべく応急処置的に実装したものです。
      memcpy(&_file, &file, sizeof(fs::File));
      // memsetにより一時変数の中身を吹っ飛ばし、デストラクタによるcloseを予防します。
      memset(&file, 0, sizeof(fs::File));
      _fp = &_file;
      return _file;
    }

    int read(std::uint8_t *buf, std::uint32_t len) override { return _fp->read(buf, len); }
    void skip(std::int32_t offset) override { seek(offset, SeekCur); }
    bool seek(std::uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(std::uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close() override { _fp->close(); }
    std::int32_t tell(void) override { return _fp->position(); }

#else  // dummy.

    bool open(const char*) override { return false; }
    int read(std::uint8_t*, std::uint32_t) override { return 0; }
    void skip(std::int32_t) override { }
    bool seek(std::uint32_t) override { return false; }
    bool seek(std::uint32_t, int) { return false; }
    void close() override { }
    std::int32_t tell(void) override { return 0; }

#endif

  };

//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (Stream_h)

  struct StreamWrapper : public DataWrapper
  {
    void set(Stream* src, std::uint32_t length = ~0u) { _stream = src; _length = length; _index = 0; }

    int read(std::uint8_t *buf, std::uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(std::int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(std::uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }
    std::int32_t tell(void) override { return _index; }

  private:
    Stream* _stream;
    std::uint32_t _index;
    std::uint32_t _length = 0;

  };

#endif

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace spi
  {
    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi);
    void release(int spi_host);
    void beginTransaction(int spi_host, std::uint32_t freq, int spi_mode = 0);
    void beginTransaction(int spi_host);
    void endTransaction(int spi_host);
    void writeBytes(int spi_host, const std::uint8_t* data, std::size_t length);
    void readBytes(int spi_host, std::uint8_t* data, std::size_t length);
  }

  /// unimplemented.
  namespace i2c
  {
    static constexpr std::uint32_t I2C_DEFAULT_FREQ = 400000;

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl);
    cpp::result<void, error_t> release(int i2c_port);
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, std::uint32_t freq, bool read = false);
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, std::uint32_t freq, bool read = false);
    cpp::result<void, error_t> endTransaction(int i2c_port);
    cpp::result<void, error_t> writeBytes(int i2c_port, const std::uint8_t *data, std::size_t length);
    cpp::result<void, error_t> readBytes(int i2c_port, std::uint8_t *data, std::size_t length);

//--------

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, std::uint8_t *readdata, std::uint8_t readlen, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::size_t readlen, std::uint32_t freq = I2C_DEFAULT_FREQ);

    cpp::result<std::uint8_t, error_t> registerRead8(int i2c_port, int addr, std::uint8_t reg, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> registerWrite8(int i2c_port, int addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask = 0, std::uint32_t freq = I2C_DEFAULT_FREQ);

    inline cpp::result<void, error_t> registerRead(int i2c_port, int addr, std::uint8_t reg, std::uint8_t* data, std::size_t len, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return transactionWriteRead(i2c_port, addr, &reg, 1, data, len, freq);
    }
    inline cpp::result<void, error_t> bitOn(int i2c_port, int addr, std::uint8_t reg, std::uint8_t bit, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return registerWrite8(i2c_port, addr, reg, bit, ~0, freq);
    }
    inline cpp::result<void, error_t> bitOff(int i2c_port, int addr, std::uint8_t reg, std::uint8_t bit, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return registerWrite8(i2c_port, addr, reg, 0, ~bit, freq);
    }
  }

//----------------------------------------------------------------------------
 }
}
