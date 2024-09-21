#include <span>
#include <stdexcept>

#include "Mmu.hpp"

Mmu::Mmu(Mmu& other) {
    auto size = other.memory.size();

    memory = other.memory;
    permission = other.permission;
    dirty.reserve(size / DIRTY_BLOCK_SIZE + 1);

    dirty_bitmap.resize(size / DIRTY_BLOCK_SIZE / 64 + 1, 0);
    cur_alloc = other.cur_alloc;
}

void Mmu::SetPermission(VirtualAddr addr, size_t size, Permission perm) {
    if (addr >= permission.size()) {
        throw std::out_of_range("Starting address is out of bounds");
    }

    size_t end = addr + size;
    if (end > permission.size()) {
        throw std::out_of_range("End address exceeds bounds");
    }

    std::fill(permission.begin() + addr, permission.begin() + end, perm);
}

void Mmu::WriteFrom(VirtualAddr addr, const std::vector<u8>& buf) {
    size_t end = addr + buf.size();

    if (addr >= memory.size()) 
        throw std::out_of_range("Starting address is out of bounds");

    if (end > memory.size()) 
        throw std::out_of_range("End address exceeds bounds");

    bool has_raw = false;
    for (size_t i = addr; i < end; ++i) {
        if ((permission[i] & PERM_WRITE) == 0) 
            throw std::runtime_error("WriteFrom: Access violation");
        has_raw |= (permission[i] & PERM_RAW) != 0;
    }

    //Compute dirty bit blocks
    auto block_start = addr / DIRTY_BLOCK_SIZE;
    auto block_end   = (addr + buf.size()) / DIRTY_BLOCK_SIZE;
    for (auto block = block_start; block <= block_end; ++block) {
        //Determine the bitmap position of the dirty block
        auto idx = block_start / 64;
        auto bit = block_start % 64;

        //Check if the block is not dirty
        if ((dirty_bitmap[idx] & (1 << bit)) == 0) {
            //Block is not dirty, add it to the dirty list
            dirty.push_back(block);

            //Update the dirty bitmap
            dirty_bitmap[idx] |= 1 << bit;
        }
    }

    if (has_raw) {
        for (size_t i = addr; i < end; ++i) {
            if ((permission[i] & PERM_RAW) != 0) 
                permission[i] |= PERM_READ;
        }
    }

    std::copy(buf.begin(), buf.end(), memory.begin() + addr);
}

void Mmu::ReadIntoPerm(VirtualAddr addr, std::vector<u8>& buf, Permission expected_perm) {
    // Check if the address and size are within bounds
    if (addr >= memory.size()) 
        throw std::out_of_range("Starting address is out of bounds");

    size_t end = addr + buf.size();
    if (end > memory.size()) 
        throw std::out_of_range("End address exceeds bounds");

    // Check read permissions for the specific range
    if (expected_perm != 0) {
        for (size_t i = addr; i < end; ++i) {
            if ((permission[i] & expected_perm) != expected_perm)
                throw std::runtime_error("ReadInto: Access violation");
        }
    }

    // Ensure the buffer has enough space
    if (buf.size() < (end - addr)) 
        buf.resize(end - addr);

    // Copy data from memory to the buffer
    std::copy(memory.begin() + addr, memory.begin() + end, buf.begin());
}


void Mmu::ReadInto(VirtualAddr addr, std::vector<u8>& buf) {
    ReadIntoPerm(addr, buf, PERM_READ);
}

void Mmu::ReadInstruction(Ldasm& lendec, VirtualAddr addr, std::vector<u8>& buf, Permission exp_perm) {
    // Decode the instruction length
    size_t instr_length = lendec.DecodeInstructionLength(DasmMode::X86_Dmode_64bit, memory, addr);

    // Resize the buffer to the exact length needed
    buf.resize(instr_length);
    ReadIntoPerm(addr, buf, exp_perm);
}

bzmu::result<VirtualAddr, memalloc_error> Mmu::mem_alloc(size_t size) {
    size_t align_size = (size + 0xf) & ~0xf; // Align to 16 bytes

    size_t end_alloc = cur_alloc + align_size;
    if (end_alloc > memory.size()) {
        return bzmu::result_error{ memalloc_error::no_space }; // Not enough space
    }

    VirtualAddr base = cur_alloc;
    cur_alloc = end_alloc;

    SetPermission(base, align_size, PERM_RAW | PERM_WRITE);

    return base;
}

void Mmu::Reset(const Mmu& other) {
    for (auto& block : dirty) {
        auto start = block * DIRTY_BLOCK_SIZE;
        auto end = (block + 1) * DIRTY_BLOCK_SIZE;

        //Zero the bitmap
        dirty_bitmap[block / 64] = 0;

        std::copy(other.memory.begin() + start, other.memory.begin() + end, memory.begin() + start);
        std::copy(other.permission.begin() + start, other.permission.begin() + end, permission.begin() + start);
    }

    //Clear the dirty list
    dirty.clear();
}