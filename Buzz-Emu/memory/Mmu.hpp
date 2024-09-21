#pragma once

#include <vector>
#include <optional>

#include "Block.hpp"
#include "Permission.hpp"
#include "VirtualAddr.hpp"
#include "../include/Results.hpp"
#include "../emulation/x86/Decoder.hpp"

enum class memalloc_error {
    no_space = 0,
};

/* An isolated memory space */
struct Mmu {
    std::vector<u8> memory;
    std::vector<Permission> permission;
    /*
    Tracks block in memory which are dirty
    */
    std::vector<size_t>     dirty;

    /*
    Trackes which part of the memory have been dirtied
    */
    std::vector<u64> dirty_bitmap;

    VirtualAddr cur_alloc;

    Mmu(size_t size) : memory(size), permission(size, Permission(0)), dirty_bitmap(size / DIRTY_BLOCK_SIZE / 64 + 1, 0), cur_alloc(0x10000) {
        dirty.reserve(size / DIRTY_BLOCK_SIZE + 1);
    }

    Mmu(Mmu& other);

    [[nodiscard]] bzmu::result<VirtualAddr, memalloc_error> mem_alloc(size_t size);
    void SetPermission(VirtualAddr addr, size_t size, Permission perm);

    void WriteFrom(VirtualAddr addr, const std::vector<u8>& buf);
    void ReadInto(VirtualAddr addr, std::vector<u8>& buf);
    void ReadIntoPerm(VirtualAddr addr, std::vector<u8>& buf, Permission expected_perm);
    void ReadInstruction(Ldasm& lendec, VirtualAddr addr, std::vector<u8>& buf, Permission exp_perm);

    template<typename T> requires std::is_trivially_copyable_v<T>
    std::optional<T> ReadPerm(VirtualAddr addr, Permission exp_perm) {
        T tmp{};
        std::vector<u8> buf(sizeof(T));
        ReadIntoPerm(addr, buf, exp_perm);
        std::memcpy(&tmp, buf.data(), sizeof(T));
        return tmp;
    }

    template<typename T> requires std::is_trivially_copyable_v<T>
    std::optional<T> Read(const VirtualAddr& addr) {
        return ReadPerm<T>(addr, PERM_READ);
    }

    template<typename T> requires std::is_trivially_copyable_v<T>
    void Write(VirtualAddr addr, T val) {
        std::vector<u8> buf(sizeof(T));
        std::memcpy(buf.data(), &val, sizeof(T));
        WriteFrom(addr, buf);
    }

    /*
    Restores the memory back to the original state
    (e.g. restore all dirty block to the state of `other`
    */
    void Reset(const Mmu& other);
};
