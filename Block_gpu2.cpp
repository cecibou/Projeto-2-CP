#include "Block.h"
#include "sha256.h"
#include <omp.h>

Block::Block(uint32_t nIndexIn, const string &sDataIn) : _nIndex(nIndexIn), _sData(sDataIn)
{
    _nNonce = 0;
    _tTime = time(nullptr);

    sHash = _CalculateHash();
}

void Block::MineBlock(uint32_t nDifficulty)
{
    char cstr[nDifficulty + 1];
    #pragma omp target teams distribute parallel for
    for (uint32_t i = 0; i < nDifficulty; ++i)
    {
        cstr[i] = '0';
    }
    cstr[nDifficulty] = '\0';

    string str(cstr);

    #pragma omp target teams distribute parallel
    {
        do
        {
            #pragma omp critical
            {
                _nNonce++;
                sHash = _CalculateHash();
            }
        } while (sHash.substr(0, nDifficulty) != str);
    }

    cout << "Block mined: " << sHash << endl;
}

inline string Block::_CalculateHash() const
{
    stringstream ss;
    ss << _nIndex << sPrevHash << _tTime << _sData << _nNonce;

    return sha256(ss.str());
}
