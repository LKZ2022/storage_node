//Copyright (c) 2026 Liu Kaizhi
//Licensed under the Apache License, Version 2.0.


#include "../include/SocketFD.hpp"

#include <system_error>

namespace ref_storage::net {

    SocketFD::SocketFD() noexcept = default;

    SocketFD::SocketFD(NativeSocketType fd) noexcept : _handle(fd) {}

    SocketFD & SocketFD::operator=(SocketFD &&other) noexcept {
        if (this != &other) {
            close_handle();
            _handle = std::exchange(other._handle, kInvalid);
        }
        return *this;
    }

    SocketFD::~SocketFD() noexcept {
        close_handle();
    }

    SocketFD::NativeSocketType SocketFD::native_handle() const noexcept {
        return _handle;
    }

    SocketFD::operator bool() const noexcept {
        return _handle != kInvalid;
    }

    bool SocketFD::is_valid_handle() const noexcept {
        return static_cast<bool>(*this);
    }

    bool SocketFD::operator==(const SocketFD &other) const noexcept {
        return _handle == other._handle;
    }

    bool SocketFD::operator!=(const SocketFD &other) const noexcept {
        return !(*this == other);
    }

    SocketFD::NativeSocketType SocketFD::release_handle() noexcept {
        return std::exchange(_handle, kInvalid);
    }

    void SocketFD::reset_handle(NativeSocketType new_handle) noexcept {
        close_handle();
        _handle = new_handle;
    }

    void SocketFD::close_handle() noexcept {
        if (is_valid_handle()) {
#ifdef _WIN32
            closesocket(_handle);
#elif __linux__
            close(_handle);
#endif
            _handle = kInvalid;
        }
    }

    int SocketFD::bind_handle(const struct sockaddr *addr, socklen_t addrlen) const {
        if (!is_valid_handle()) {
            return -1;
        }
        return bind(_handle, addr, addrlen);
    }

    int SocketFD::listen_handle(int backlog) const {
        if (!is_valid_handle()) {
            return -1;
        }
        return listen(_handle, backlog);
    }

    SocketFD SocketFD::accept_handle(struct sockaddr *addr, socklen_t *addrlen) const {
        if (!is_valid_handle()) {
            return SocketFD(kInvalid);
        }
        NativeSocketType new_handle = accept(_handle, addr, addrlen);
        return SocketFD(new_handle);
    }

    SocketFD SocketFD::create_socket_fd() {
        NativeSocketType sock = socket(AF_UNSPEC, SOCK_STREAM, 0);
        if (sock == kInvalid) {
            throw_last_error("socket() failed");
        }
        return SocketFD(sock);
    }

    void SocketFD::throw_last_error(const char *operation) {
#ifdef _WIN32
        throw std::system_error(WSAGetLastError(), std::system_category(), operation);
#elif __linux__
        throw std::system_error(errno, std::system_category(), operation);
#endif
    }
}
