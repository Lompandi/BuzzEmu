#pragma once

/*
Block size is used for tracking and resetting memory which have been modified
the larger this is, the fewer but more expensive memcpys() need to occur
the small, the greater but less expensive memcpys() need to occur
sweetspot: 128-4096 bytes
*/
constexpr size_t DIRTY_BLOCK_SIZE = 4096;
