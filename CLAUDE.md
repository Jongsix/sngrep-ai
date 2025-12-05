# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

sngrep is a terminal-based SIP (Session Initiation Protocol) messages flow viewer written in C. It supports live packet capture and PCAP file analysis, displaying SIP call flows with an ncurses-based interface.

## Build System

This project supports two build systems:

### Autotools (Primary)
```bash
./bootstrap.sh
./configure [options]
make
make install  # as root
```

### CMake (Alternative)
```bash
mkdir build && cd build
cmake [options] ..
make
make install  # as root
```

## Common Build Options

Both build systems support these features via flags:

**Autotools:**
- `--with-openssl` - TLS/SSL support via OpenSSL
- `--with-gnutls` - TLS/SSL support via GnuTLS (mutually exclusive with OpenSSL)
- `--with-pcre` - PCRE regex support
- `--with-pcre2` - PCRE2 regex support (mutually exclusive with PCRE)
- `--with-zlib` - Compressed PCAP support
- `--enable-unicode` - Wide character/UTF-8 support
- `--enable-ipv6` - IPv6 packet capture
- `--enable-eep` - EEP/HEP protocol support
- `--enable-debug` - Debug build with symbols

**CMake:**
- `-D WITH_OPENSSL=ON`
- `-D WITH_GNUTLS=ON`
- `-D WITH_PCRE=ON`
- `-D WITH_PCRE2=ON`
- `-D WITH_ZLIB=ON`
- `-D WITH_UNICODE=ON`
- `-D USE_IPV6=ON`
- `-D USE_EEP=ON`

## Testing

### Running All Tests (Autotools)
```bash
make check
```

### Running All Tests (CMake)
```bash
cd build
make tests   # builds test binaries
ctest        # runs tests
```

### Running Individual Tests
Test binaries are located in `tests/` and numbered `test_001` through `test_011`. After building, run them directly:
```bash
./tests/test-001  # autotools
./build/test_001  # cmake
```

## Architecture

### Core Components

**Packet Capture Layer** (`src/capture.c`, `src/capture.h`)
- Main packet capture engine using libpcap
- Thread-based architecture for concurrent capture and display
- Handles packet filtering and dispatching
- Protocol-specific implementations:
  - `capture_gnutls.c` - TLS decryption via GnuTLS
  - `capture_openssl.c` - TLS decryption via OpenSSL
  - `capture_eep.c` - EEP/HEP encapsulation protocol

**SIP Protocol Processing** (`src/sip.c`, `src/sip_call.c`, `src/sip_msg.c`)
- `sip.c` - Core SIP parser and message manager
- `sip_call.c` - Call state tracking and dialog management
- `sip_msg.c` - Individual SIP message representation
- `sip_attr.c` - SIP header attribute extraction

**Data Structures**
- `src/vector.c` - Dynamic array implementation used throughout
- `src/hash.c` - Hash table for efficient Call-ID lookups
- `src/group.c` - Call grouping by various criteria

**RTP Support** (`src/rtp.c`)
- RTP packet parsing and payload capture
- Media stream tracking and statistics

**UI Layer** (`src/curses/`)
- Panel-based architecture with ncurses
- `ui_manager.c` - Central UI coordinator and panel navigation
- `ui_call_list.c` - Main call list view (primary interface)
- `ui_call_flow.c` - SIP ladder/flow diagram view
- `ui_call_raw.c` - Raw SIP message viewer
- `ui_stats.c` - Call statistics panel
- `ui_filter.c` - Filtering dialog
- `ui_save.c` - PCAP save dialog
- `ui_msg_diff.c` - Message comparison view
- `ui_column_select.c` - Column customization
- `ui_settings.c` - Settings editor
- `ui_panel.c` - Base panel infrastructure
- `scrollbar.c` - Scrollbar widget

### Configuration System

**Settings** (`src/setting.c`)
- Runtime configuration via sngreprc file
- Located in `/etc/sngreprc` or `~/.sngreprc`
- Key-value pairs for UI and behavior customization

**Key Bindings** (`src/keybinding.c`)
- Configurable keyboard shortcuts
- Vim-style navigation support

**Filtering** (`src/filter.c`)
- Message filtering with regex support (PCRE/PCRE2 if enabled)
- BPF filter support for packet capture

## Key Data Flow

1. `main.c` initializes capture and UI subsystems
2. `capture.c` spawns packet capture thread
3. Packets are parsed by `sip.c` and organized into calls via `sip_call.c`
4. Calls stored in vector/hash structures for efficient access
5. UI panels (`ui_call_list.c`, etc.) display and navigate the call data
6. User interactions flow through `ui_manager.c` to appropriate panels

## Conditional Compilation

Many features are controlled via preprocessor directives:
- `WITH_GNUTLS` / `WITH_OPENSSL` - TLS decryption
- `WITH_PCRE` / `WITH_PCRE2` - Regex support
- `WITH_UNICODE` - Wide character support
- `USE_IPV6` - IPv6 support
- `USE_EEP` - EEP/HEP support
- `WITH_ZLIB` - Compressed PCAP support

These are set during configure/cmake and defined in `src/config.h`.

## Dependencies

**Required:**
- libpcap - packet capture
- libncurses - terminal UI
- libpthread - threading
- ncurses panel/form/menu libraries

**Optional:**
- libssl/libcrypto (OpenSSL) OR gnutls/libgcrypt - TLS decryption
- libpcre or libpcre2 - advanced regex
- zlib - compressed PCAP files
- libncursesw - wide character support
