#include "Block.h"
#include "sha256.h"

Block::Block(uint32_t nIndexIn, const string &sDataIn) : _nIndex(nIndexIn), _sData(sDataIn)
{
    _nNonce = 0;
    _tTime = time(nullptr);

    sHash = _CalculateHash();
}

void Block::MineBlock(uint32_t nDifficulty)
{
    char cstr[nDifficulty + 1];

    #pragma omp parallel for 
    for (uint32_t i = 0; i < nDifficulty; ++i)
    {
        cstr[i] = '0';
    }
    cstr[nDifficulty] = '\0';

    string str(cstr);

    #pragma omp parallel
	{
		#pragma omp single nowait
		{
			while (sHash.substr(0, nDifficulty) != str)
			{
				#pragma omp task firstprivate(sHash)
				_nNonce++;
				#pragma omp critical(sHash)
				sHash = _CalculateHash();
			}
		}
	}

    cout << "Block mined: " << sHash << endl;
}

inline string Block::_CalculateHash() const
{
    stringstream ss;

    #pragma critical 
    ss << _nIndex << sPrevHash << _tTime << _sData << _nNonce;

    return sha256(ss.str());
}

/*

1- Paralelização da Inicialização da Dificuldade:

#pragma omp parallel for 
for (uint32_t i = 0; i < nDifficulty; ++i)
{
    cstr[i] = '0';
}

Neste trecho, a inicialização da string de dificuldade foi paralelizada usando OpenMP. Cada thread paralela preenche parte da string cstr com zeros.

2- Loop de Mineração Paralelizado:

#pragma omp parallel
{
    #pragma omp single nowait
    while (sHash.substr(0, nDifficulty) != str)
    {
        #pragma omp task firstprivate(sHash)
        _nNonce++;
        #pragma omp critical(sHash)
        sHash = _CalculateHash();
    }
}


O loop de mineração foi paralelizado. Cada thread paralela tenta encontrar um hash que atenda à dificuldade necessária, e tarefas paralelas são criadas para incrementar _nNonce e calcular o novo hash.

3- Seção Crítica para o Cálculo do Hash:

#pragma critical 
ss << _nIndex << sPrevHash << _tTime << _sData << _nNonce;

A função _CalculateHash inclui uma seção crítica para garantir que o cálculo do hash seja realizado por apenas uma thread de cada vez. Isso é necessário para evitar condições de corrida, já que várias threads podem estar incrementando _nNonce simultaneamente.

4- Tarefa Firstprivate:

#pragma omp task firstprivate(sHash)

A cláusula firstprivate garante que cada tarefa tenha sua cópia privada da variável sHash. Isso é necessário para evitar corridas de dados na variável sHash quando várias threads estão executando as tarefas simultaneamente.


*/