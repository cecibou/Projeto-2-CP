// Block.cu
#include "Block.h"
#include "sha256.h"
#include <iostream>

__global__ void mineBlockKernel(uint32_t difficulty, uint32_t* nonce, char* hash)
{
    char cstr[difficulty + 1];
    for (uint32_t i = 0; i < difficulty; ++i)
    {
        cstr[i] = '0';
    }
    cstr[difficulty] = '\0';

    string str(cstr);

    do
    {
        atomicAdd(nonce, 1);
        stringstream ss;
        ss << blockIdx.x << sPrevHash << time(nullptr) << _sData << *nonce;
        sha256(ss.str().c_str(), hash);

    } while (hash[0] != str[0] || hash[1] != str[1] || hash[2] != str[2]); // Adapt this for your difficulty

    std::cout << "Block mined: " << hash << std::endl;
}

Block::Block(uint32_t nIndexIn, const string& sDataIn) : _nIndex(nIndexIn), _sData(sDataIn)
{
    _nNonce = 0;
    _tTime = time(nullptr);
    sHash = _CalculateHash();
}

void Block::MineBlock(uint32_t nDifficulty)
{
    char* d_hash;
    uint32_t* d_nonce;

    cudaMalloc((void**)&d_hash, SHA256_BLOCK_SIZE);
    cudaMalloc((void**)&d_nonce, sizeof(uint32_t));

    cudaMemcpy(d_hash, sHash.c_str(), SHA256_BLOCK_SIZE, cudaMemcpyHostToDevice);
    cudaMemcpy(d_nonce, &_nNonce, sizeof(uint32_t), cudaMemcpyHostToDevice);

    mineBlockKernel<<<1, 1>>>(nDifficulty, d_nonce, d_hash);

    cudaMemcpy(sHash.data(), d_hash, SHA256_BLOCK_SIZE, cudaMemcpyDeviceToHost);
    cudaMemcpy(&_nNonce, d_nonce, sizeof(uint32_t), cudaMemcpyDeviceToHost);

    cudaFree(d_hash);
    cudaFree(d_nonce);
}

inline string Block::_CalculateHash() const
{
    stringstream ss;
    ss << _nIndex << sPrevHash << _tTime << _sData << _nNonce;
    return sha256(ss.str());
}
