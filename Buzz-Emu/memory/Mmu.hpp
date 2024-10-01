#pragma once

#include <vector>
#include <optional>

#include "Block.hpp"
#include "Permission.hpp"
#include "VirtualAddr.hpp"
#include "../emulator/VmExit.hpp"
#include "../emulation/x86/Decoder.hpp"
#include "../include/buzzemu/Results.hpp"

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

    bzmu::result<int, VmExitResult> WriteFrom(VirtualAddr addr, const std::vector<u8>& buf);
    bzmu::result<int, VmExitResult> ReadInto(VirtualAddr addr, std::vector<u8>& buf);
    bzmu::result<int, VmExitResult> ReadIntoPerm(VirtualAddr addr, std::vector<u8>& buf, Permission expected_perm);
    void ReadInstruction(Ldasm& lendec, VirtualAddr addr, std::vector<u8>& buf, Permission exp_perm);

    template<typename T>
        requires std::is_trivially_copyable_v<T>
    bzmu::result<T, VmExitResult> ReadPerm(VirtualAddr addr, Permission exp_perm) {
        std::vector<u8> buf(sizeof(T));
        auto result = ReadIntoPerm(addr, buf, exp_perm);

        if (!result.has_value())
            return bzmu::result_error{ result.error() };
        T tmp{};
        std::memcpy(&tmp, buf.data(), sizeof(T));
        return tmp;
    }

    template<typename T> requires std::is_trivially_copyable_v<T>
    bzmu::result<T, VmExitResult> Read(const VirtualAddr& addr) {
        return ReadPerm<T>(addr, PERM_READ);
    }

    template<typename T> requires std::is_trivially_copyable_v<T>
    bzmu::result<int, VmExitResult> Write(VirtualAddr addr, T val) {
        std::vector<u8> buf(sizeof(T));
        std::memcpy(buf.data(), &val, sizeof(T));
        return WriteFrom(addr, buf);
    }

    /*
    Return a slice to memory at `addr` for `size` bytes that 
    has been vaidated to match all `exp_perms`
    */
    bzmu::result<std::vector<u8>, VmExitResult>
    peek_perms(VirtualAddr addr, size_t size, Permission exp_perms);

    /*
    Restores the memory back to the original state
    (e.g. restore all dirty block to the state of `other`
    */
    void Reset(const Mmu& other);
};
