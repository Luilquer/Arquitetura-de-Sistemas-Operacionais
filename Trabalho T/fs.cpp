

/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3


 g++ -std=c++17 fs.cpp main.cpp sha256.cpp -o test -lgtest -lcrypto -lpthread


 */

// Carlos Luilquer Almeida Santos (20150465)

#include "fs.h"
#include "math.h"
#include <string.h>

void lerValores(char *tamanhoDoBloco, char *numDeBlocos, char *numInodes, std::string fsFileName)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    fread(tamanhoDoBloco, sizeof(char), 1, file);
    fread(numDeBlocos, sizeof(char), 1, file);
    fread(numInodes, sizeof(char), 1, file);

    fclose(file);
}

int inodeLivreIndex(std::string fsFileName, char numDeBlocos, char numInodes)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    INODE inode;
    int inodeLivre;
    char tamanhoMapaDeBit = ceil(numDeBlocos / 8.0);

    int vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);

        if (inode.IS_USED == 0)
        {
            inodeLivre = i;
            break;
        }
    }
    fclose(file);
    return inodeLivre;
}

int blocoLivreIndex(std::string fsFileName, char tamanhoMapaDeBit, char numInodes, INODE inode)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");
    char indexBlocoLivre = 0;

    int vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);

        for (size_t j = 0; j < sizeof(inode.DIRECT_BLOCKS); j++)
        {
            if (inode.DIRECT_BLOCKS[j] > indexBlocoLivre)
            {
                indexBlocoLivre = inode.DIRECT_BLOCKS[j];
            }
        }
    }
    indexBlocoLivre++;

    fclose(file);

    return indexBlocoLivre;
}

void seForBarra(std::string fsFileName, char tamanhoMapaDeBit, char numInodes, char tamanhoDoBloco, char indexBlocoLivre)
{

    FILE *file = fopen(fsFileName.c_str(), "r+");

    char directoryBlock[] = {1, 2};
    int vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1;
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&directoryBlock, sizeof(char), 2, file);

    fclose(file);
}

char quantidadeBlocosUsados(std::string fsFileName, char tamanhoMapaDeBit, char numInodes, INODE inode)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");
    char numeroBlocosUsados = 0;

    int vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);

        for (size_t j = 0; j < sizeof(inode.DIRECT_BLOCKS); j++)
        {
            if (inode.DIRECT_BLOCKS[j] > numeroBlocosUsados)
            {
                numeroBlocosUsados = inode.DIRECT_BLOCKS[j];
            }
        }
    }
    fclose(file);

    return numeroBlocosUsados;
}

void adicionarBitMap(std::string fsFileName, char numeroDeBlocos)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    int mapaDeBit = 0;
    for (size_t i = 0; i < numeroDeBlocos; i++)
    {
        mapaDeBit += pow(2, i);
    }

    fseek(file, 3, SEEK_SET);
    fwrite(&mapaDeBit, sizeof(char), 1, file);
    fclose(file);
}

void inodeVazio(std::string fsFileName, char tamanhoMapaDeBit, char indexDoCaminho, char indexDoDiretorio)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    int vaiPara = 3 + tamanhoMapaDeBit + sizeof(INODE) * indexDoCaminho;
    char emptyInode[22];
    for (size_t i = 0; i < sizeof(INODE); i++)
    {
        emptyInode[i] = 0;
    }
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&emptyInode, sizeof(INODE), 1, file);

    char tamanhoDiretorio = 0;
    vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorio * sizeof(INODE) + 12;
    fseek(file, vaiPara, SEEK_SET);
    fread(&tamanhoDiretorio, sizeof(char), 1, file);
    fseek(file, -1, SEEK_CUR);
    tamanhoDiretorio--;
    fwrite(&tamanhoDiretorio, sizeof(char), 1, file);

    fclose(file);
}

void initFs(std::string fsFileName, int tamanhoDoBloco, int numDeBlocos, int numInodes)
{
    FILE *file = fopen(fsFileName.c_str(), "w+");

    fwrite(&tamanhoDoBloco, sizeof(char), 1, file);
    fwrite(&numDeBlocos, sizeof(char), 1, file);
    fwrite(&numInodes, sizeof(char), 1, file);

    char mapaDeBit = 1;
    fwrite(&mapaDeBit, sizeof(char), 1, file);

    char vazio[(numInodes - 1) * sizeof(INODE)];

    for (int i = 0; i < sizeof(vazio); ++i)
    {
        vazio[i] = 0;
    }

    fwrite(&vazio, sizeof(char), (numDeBlocos - 1) / 8, file);

    INODE root = {
        1,
        1,
        "/",
        0,
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
    };

    fwrite(&root, sizeof(char), sizeof(INODE), file);
    fwrite(&vazio, sizeof(char), (numInodes - 1) * sizeof(INODE), file);
    fwrite(&vazio, sizeof(char), 1, file);
    fwrite(&vazio, sizeof(char), numDeBlocos * tamanhoDoBloco, file);

    fclose(file);
}

void addFile(std::string fsFileName, std::string filePath, std::string fileContent)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    char tamanhoDoBloco, numDeBlocos, numInodes;
    lerValores(&tamanhoDoBloco, &numDeBlocos, &numInodes, fsFileName);

    INODE inode;
    int inodeLivre = inodeLivreIndex(fsFileName, numDeBlocos, numInodes);
    char tamanhoMapaDeBit = ceil(numDeBlocos / 8.0);
    char nomeDiretorio[10], nomeCaminho[10];

    int contadorDeDiretorio = 0, indexBarra = 0;

    for (size_t i = 0; i < strlen(filePath.c_str()); i++)
    {
        if (filePath.at(i) == '/')
        {
            contadorDeDiretorio++;
            indexBarra = i;
        }
    }

    bool bloquearNomeDiretorio = false, bloquearCaminhoNome = false;
    for (size_t i = 0; i < sizeof(nomeCaminho); i++)
    {
        if (contadorDeDiretorio == 1)
        {
            if ((i + 1) < strlen(filePath.c_str()))
                nomeCaminho[i] = filePath.at(i + 1);
            else
                nomeCaminho[i] = 0;

            if (filePath.at(i) == '/')
                nomeDiretorio[i] = '/';
            else
                nomeDiretorio[i] = 0;
        }

        else
        {
            if ((i + 1) < strlen(filePath.c_str()) && filePath.at(i + 1) != '/' && !bloquearNomeDiretorio)
                nomeDiretorio[i] = filePath.at(i + 1);
            else
            {
                nomeDiretorio[i] = 0;
                bloquearNomeDiretorio = true;
            }

            if ((indexBarra + 1) < strlen(filePath.c_str()) && filePath.at(indexBarra + 1) != '/' && !bloquearCaminhoNome)
                nomeCaminho[i] = filePath.at(indexBarra + 1);
            else
            {
                nomeCaminho[i] = 0;
                bloquearCaminhoNome = true;
            }
            indexBarra++;
        }
    }

    int vaiPara = numInodes * sizeof(INODE) + 3 + tamanhoMapaDeBit + 1;
    fseek(file, vaiPara, SEEK_SET);

    char bloquearValores[numDeBlocos * tamanhoDoBloco];
    fread(&bloquearValores, sizeof(char), sizeof(bloquearValores), file);

    if (nomeDiretorio[0] != '/')
    {
        vaiPara = 3 + tamanhoMapaDeBit;
        fseek(file, vaiPara, SEEK_SET); //

        for (int i = 0; i < numInodes; i++)
        {
            fread(&inode, sizeof(INODE), 1, file);
            if (!strcmp(inode.NAME, nomeDiretorio))
            {
                char tamanhoDiretorio = 0;
                vaiPara = 3 + tamanhoMapaDeBit + i * sizeof(INODE) + 12;
                fseek(file, vaiPara, SEEK_SET); ///--
                fread(&tamanhoDiretorio, sizeof(char), 1, file);
                fseek(file, -1, SEEK_CUR);
                tamanhoDiretorio++;                               //
                fwrite(&tamanhoDiretorio, sizeof(char), 1, file); //
                vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1 + tamanhoDoBloco * inode.DIRECT_BLOCKS[0];
                fseek(file, vaiPara, SEEK_SET);
                fwrite(&inodeLivre, sizeof(char), 1, file); //
                vaiPara = 3 + tamanhoMapaDeBit + i * sizeof(INODE) + sizeof(INODE);
                fseek(file, vaiPara, SEEK_SET);
            }
        }
    }

    char indexBlocoLivre = blocoLivreIndex(fsFileName, tamanhoMapaDeBit, numInodes, inode);

    char definirInodeUsado[2] = {0x01, 0x00};
    vaiPara = 3 + tamanhoMapaDeBit + sizeof(INODE) * inodeLivre;
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&definirInodeUsado, sizeof(char), 2, file);
    fwrite(&nomeCaminho, sizeof(char), 10, file);

    int tamanhoConteudo = strlen(fileContent.c_str());
    fwrite(&tamanhoConteudo, sizeof(char), 1, file);

    char numeroDeBlocos = ceil(tamanhoConteudo / (double)tamanhoDoBloco);

    for (size_t i = 0; i < numeroDeBlocos; i++)
    {
        fwrite(&indexBlocoLivre, sizeof(char), 1, file);
        indexBlocoLivre++;
    }

    char conteudoDoAqruivo[strlen(fileContent.c_str())];
    for (size_t i = 0; i < strlen(fileContent.c_str()); i++)
    {
        conteudoDoAqruivo[i] = fileContent.at(i);
    }

    if (nomeDiretorio[0] != '/')
    {
        seForBarra(fsFileName, tamanhoMapaDeBit, numInodes, tamanhoDoBloco, indexBlocoLivre);

        vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1 + tamanhoDoBloco * (indexBlocoLivre - numeroDeBlocos);
        fseek(file, vaiPara, SEEK_SET);
        fwrite(&conteudoDoAqruivo, sizeof(char), strlen(fileContent.c_str()), file);
    }

    else
    {
        char directoryBlock[] = {1, 0};
        vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1;
        fseek(file, vaiPara, SEEK_SET);
        fwrite(&directoryBlock, sizeof(char), 2, file);
        fwrite(&conteudoDoAqruivo, sizeof(char), strlen(fileContent.c_str()), file);
    }

    char numeroBlocosUsados = quantidadeBlocosUsados(fsFileName, tamanhoMapaDeBit, numInodes, inode);

    int valorParaHex = 0;
    int quantidadePreenchido = 0;
    for (size_t i = 0; i <= numeroBlocosUsados; i++)
    {

        valorParaHex += pow(2, quantidadePreenchido);
        quantidadePreenchido++;
    }
    fseek(file, 3, SEEK_SET);
    fwrite(&valorParaHex, sizeof(char), 1, file);

    int numeroDeFilhos = 1;
    if (nomeDiretorio[0] != '/')
    {
        numeroDeFilhos = 2;
    }
    vaiPara = 3 + tamanhoMapaDeBit + 12;
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&numeroDeFilhos, sizeof(char), 1, file);

    fclose(file);
}

void addDir(std::string fsFileName, std::string dirPath)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    char tamanhoDoBloco, numDeBlocos, numInodes;
    lerValores(&tamanhoDoBloco, &numDeBlocos, &numInodes, fsFileName);

    char tamanhoMapaDeBit = ceil((numDeBlocos - 1) / 8.0); //
    char bloquearValores[numDeBlocos * tamanhoDoBloco];
    int vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1;
    fseek(file, vaiPara, SEEK_SET);
    fread(&bloquearValores, sizeof(char), sizeof(bloquearValores), file);

    INODE inode;
    int inodeLivre = inodeLivreIndex(fsFileName, numDeBlocos, numInodes);  
    vaiPara = 3 + tamanhoMapaDeBit;

    char dirName[10]; 
    for (size_t i = 0; i < sizeof(dirName); i++)
    {
        if (i + 1 < strlen(dirPath.c_str()) && i + 1 < strlen(dirPath.c_str()))
            dirName[i] = dirPath.at(i + 1);
        else
            dirName[i] = 0;
    }

    vaiPara = 3 + tamanhoMapaDeBit + inodeLivre * sizeof(INODE);
    char setDirectory[] = {0x01, 0x01};

    // Aqui defini como diretorio
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&setDirectory, sizeof(char), 2, file);
    fwrite(&dirName, sizeof(char), 10, file);

    fseek(file, 1, SEEK_CUR);
    int indexBlocoLivre;
    for (size_t i = 0; i < sizeof(bloquearValores); i += tamanhoDoBloco)
    {
        if (bloquearValores[i] == 0)
        {
            indexBlocoLivre = i / 2; //
            break;
        }
    }
    fwrite(&indexBlocoLivre, sizeof(char), 1, file);

    int numeroDeBlocos = 1;
    vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);
    int numeroInodesPreenchidos = -1;
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);
        if (inode.IS_USED == 1)
        {
            numeroInodesPreenchidos++;

            if (inode.DIRECT_BLOCKS[2] > 0)
            {
                numeroDeBlocos++;
            }
            if (inode.DIRECT_BLOCKS[1] > 0)
            {
                numeroDeBlocos++;
            }
            if (inode.DIRECT_BLOCKS[0] > 0)
            {
                numeroDeBlocos++;
            }
        }
    }

    int valueInRootBlock = 2;
    vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 2;
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&valueInRootBlock, sizeof(char), 1, file); //
    adicionarBitMap(fsFileName, numeroDeBlocos);
    vaiPara = 3 + tamanhoMapaDeBit + 12;
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&numeroInodesPreenchidos, sizeof(char), 1, file); //

    fclose(file);
}
// função para remover
void decrementar(std::string fsFileName, int vaiPara)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    char sizeOfDirectory = 0;
    fseek(file, vaiPara, SEEK_SET);
    fread(&sizeOfDirectory, sizeof(char), 1, file);
    fseek(file, -1, SEEK_CUR);
    sizeOfDirectory--;
    fwrite(&sizeOfDirectory, sizeof(char), 1, file);

    fclose(file);
}
void remove(std::string fsFileName, std::string path)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");

    char tamanhoDoBloco, numDeBlocos, numInodes;
    lerValores(&tamanhoDoBloco, &numDeBlocos, &numInodes, fsFileName);
    char tamanhoMapaDeBit = ceil(numDeBlocos / 8.0);

    int contadorDeDiretorio = 0, indexBarra = 0, naoFoi = 0;

    for (size_t i = 0; i < strlen(path.c_str()); i++)
    {
        if (path.at(i) == '/')
        {
            indexBarra = i;
            contadorDeDiretorio++;
        }
        else
        {
            naoFoi++;
        }
    }

    int blocoDinamicoIndex = 0;
    if (!naoFoi)
        blocoDinamicoIndex = 1;

    else
        blocoDinamicoIndex = -1;

    bool bloquearNomeDiretorio = false, bloquearCaminhoNome = false;
    char nomeDiretorio[10], nomeCaminho[10];

    // Nome do diretorio

    // Nome do caminho

    for (size_t i = 0; i < sizeof(nomeCaminho); i++)
    {
        if (contadorDeDiretorio == 1)
        {
            if ((i + 1) < strlen(path.c_str()))
                nomeCaminho[i] = path.at(i + 1);
            else
                nomeCaminho[i] = 0;

            if ((i + 1) < strlen(path.c_str()) && path.at(i) == '/')
                nomeDiretorio[i] = '/';
            else
                nomeDiretorio[i] = 0;
        }

        else
        {
            if ((i + 1) < strlen(path.c_str()) && path.at(i + 1) != '/' && !bloquearNomeDiretorio)
                nomeDiretorio[i] = path.at(i + 1);
            else
            {
                bloquearNomeDiretorio = true;
                nomeDiretorio[i] = 0;
            }

            if ((indexBarra + 1) < strlen(path.c_str()) && path.at(indexBarra + 1) != '/' && !bloquearCaminhoNome)
                nomeCaminho[i] = path.at(indexBarra + 1);
            else
            {
                bloquearCaminhoNome = true;
                nomeCaminho[i] = 0;
            }
            ++indexBarra;
        }
    }

    INODE inode;
    int vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET); //
    int indexDoCaminho = 0, indexDoDiretorio = 0;
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);

        if (!strcmp(inode.NAME, nomeCaminho))
            indexDoCaminho = i;

        if (!strcmp(inode.NAME, nomeDiretorio))
            indexDoDiretorio = i;
    }

    // Inode vazio e definindo tamanho
     vaiPara = 3 + tamanhoMapaDeBit + sizeof(INODE) * indexDoCaminho;
    char inodeVazio[22];
    for (size_t i = 0; i < sizeof(INODE); i++)
    {
        inodeVazio[i] = 0;
    }
    fseek(file, vaiPara, SEEK_SET);
    fwrite(&inodeVazio, sizeof(INODE), 1, file);

    vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorio * sizeof(INODE) + 12;
    decrementar(fsFileName, vaiPara);


    INODE novoInode;

    char valorDoBitMap[numDeBlocos];
    for (size_t i = 0; i < sizeof(valorDoBitMap); i++)
    {
        if (i != 0)
            valorDoBitMap[i] = 0;
        else
            valorDoBitMap[i] = 1;
    }
    vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);

    // contar os blocos

    int diretoZero = 0;
    for (int i = 0; i < numInodes; i++)
    {
        fread(&novoInode, sizeof(INODE), 1, file);
        for (size_t j = 0; j < sizeof(novoInode.DIRECT_BLOCKS); j++)
        {
            if (novoInode.DIRECT_BLOCKS[j] != 0)
                valorDoBitMap[novoInode.DIRECT_BLOCKS[j]] = 1;
            else
            { //
                diretoZero++;
            }
        }
    }

    int valorParaHex = 0;
    if (diretoZero == 0)
    {
        // Logica do bloco
    }
    for (size_t i = 0; i < sizeof(valorDoBitMap); i++)
    {
        if (valorDoBitMap[i] != 0)

            valorParaHex += pow(2, i);
        else
            diretoZero = i;
    }

    fseek(file, 3, SEEK_SET);
    fwrite(&valorParaHex, sizeof(char), 1, file);
    int contatorInodePreenchido = -1;
    char contadorInodeUsado = 0; //
    diretoZero = contadorInodeUsado;
    vaiPara = 3 + tamanhoMapaDeBit;
    fseek(file, vaiPara, SEEK_SET);
    for (int i = 0; i < numInodes; i++)
    {
        fread(&novoInode, sizeof(INODE), 1, file);
        // lendo inode pra contar os blocos

        if (novoInode.IS_USED == 1)
        {
            contadorInodeUsado = i;
            contatorInodePreenchido++;
        }
        else
            diretoZero += i;
    }

    if (contatorInodePreenchido != 1)
 

        diretoZero = 0;
 
    else{
                vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1;
        fseek(file, vaiPara, SEEK_SET);
        fwrite(&contadorInodeUsado, sizeof(char), 1, file);
        }


    fclose(file);
}


void preencherContadorEIndex(std::string oldPath, int *contadorDeDiretorio, int *indexBarra)

{
    int contadorDeDiretorioTemp = *contadorDeDiretorio;
    int indexBarraTemp;

    for (size_t i = 0; i < strlen(oldPath.c_str()); i++)
    {
        if (oldPath.at(i) == '/')
        {
            contadorDeDiretorioTemp++;
            indexBarraTemp = i;
        }
    }

    *contadorDeDiretorio = contadorDeDiretorioTemp;
    *indexBarra = indexBarraTemp;
}

void aumentarTamanhoDiretorio(int tamanhoMapaDeBit, int indexDoDiretorioNovo, char *tamanhoDiretorio, std::string fsFileName)
{
    FILE *file = fopen(fsFileName.c_str(), "r+");
    int tamanhoDiretorioTemp = *tamanhoDiretorio;

    int vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorioNovo * sizeof(INODE) + 12;

    fseek(file, vaiPara, SEEK_SET);
    fread(&tamanhoDiretorioTemp, sizeof(char), 1, file);
    tamanhoDiretorioTemp++;
    fseek(file, -1, SEEK_CUR);
    fwrite(&tamanhoDiretorioTemp, sizeof(char), 1, file);
    *tamanhoDiretorio = tamanhoDiretorioTemp;

    fclose(file);

}

void move(std::string fsFileName, std::string oldPath, std::string newPath)
{

    FILE *file = fopen(fsFileName.c_str(), "r+");
    char tamanhoDoBloco, numDeBlocos, numInodes;
    lerValores(&tamanhoDoBloco, &numDeBlocos, &numInodes, fsFileName);

    char tamanhoMapaDeBit = ceil(numDeBlocos / 8.0);

    int contadorDeDiretorio = 0, indexBarra = 0;

    preencherContadorEIndex(oldPath, &contadorDeDiretorio, &indexBarra);

    bool bloquearNomeDiretorio = false, bloquearCaminhoNome = false;
    char nomeCaminho[10];
    char nomeDiretorio[10];
    for (size_t i = 0; i < sizeof(nomeCaminho); i++)
    {

        if (contadorDeDiretorio != 1)
        {
            if ((indexBarra + 1) < strlen(oldPath.c_str()) && oldPath.at(indexBarra + 1) != '/' && !bloquearCaminhoNome)
                nomeCaminho[i] = oldPath.at(indexBarra + 1);
            else
            {
                nomeCaminho[i] = 0;
                bloquearCaminhoNome = true;
            }
            if ((i + 1) < strlen(oldPath.c_str()) && oldPath.at(i + 1) == '/' && !bloquearNomeDiretorio)
            {
                nomeDiretorio[i] = 0;
                bloquearNomeDiretorio = true;
            }
            else
            {
                nomeDiretorio[i] = oldPath.at(i + 1);
            }

            indexBarra++;
        }

        else
        {
            if ((i + 1) < strlen(oldPath.c_str()) && oldPath.at(i) == '/')
                nomeDiretorio[i] = '/';
            else
                nomeDiretorio[i] = 0;
            if ((i + 1) >= strlen(oldPath.c_str()))
                nomeCaminho[i] = 0;
            else
                nomeCaminho[i] = oldPath.at(i + 1);
        }
    }

    contadorDeDiretorio = 0;
    indexBarra = 0;

    preencherContadorEIndex(newPath, &contadorDeDiretorio, &indexBarra);

    char novoNomeDiretorio[10], novoNomePath[10];
    bool blocNovoNomeDiretorio = false, blocNovoNomePath = false;

    for (size_t i = 0; i < sizeof(novoNomePath); i++)
    {
        if (contadorDeDiretorio != 1)
        {

            if ((i + 1) < strlen(newPath.c_str()) && newPath.at(i + 1) != '/' && !blocNovoNomeDiretorio)
            {
                novoNomeDiretorio[i] = newPath.at(i + 1);
            }
            else
            {
                novoNomeDiretorio[i] = 0;
                blocNovoNomeDiretorio = true;
            }

            if ((indexBarra + 1) < strlen(newPath.c_str()) && newPath.at(indexBarra + 1) != '/' && !blocNovoNomePath)
            {
                novoNomePath[i] = newPath.at(indexBarra + 1);
            }
            else
            {
                novoNomePath[i] = 0;
                blocNovoNomePath = true;
            }
            indexBarra++;
        }

        else
        {
            if ((i + 1) < strlen(newPath.c_str()))
                novoNomePath[i] = newPath.at(i + 1);
            else
                novoNomePath[i] = 0;

            if ((i + 1) < strlen(newPath.c_str()) && newPath.at(i) == '/')
                novoNomeDiretorio[i] = '/';
            else
                novoNomeDiretorio[i] = 0;
        }
    }

    // Achando o index
    int vaiPara = 3 + tamanhoMapaDeBit;
    int indexDoCaminho = 0, indexDoDiretorioAntigo = 0, indexDoDiretorioNovo = 0;
    INODE inode;
    fseek(file, vaiPara, SEEK_SET);
    for (int i = 0; i < numInodes; i++)
    {
        fread(&inode, sizeof(INODE), 1, file);

        if (!strcmp(inode.NAME, nomeDiretorio))
        {
            indexDoDiretorioAntigo = i;
        }

        if (!strcmp(inode.NAME, novoNomeDiretorio))
        {
            indexDoDiretorioNovo = i;
        }
        if (!strcmp(inode.NAME, nomeCaminho))
        {
            indexDoCaminho = i;
        }
    }

    if (!strcmp(nomeCaminho, novoNomePath))
    {

        vaiPara = tamanhoMapaDeBit + 3 + indexDoDiretorioAntigo * sizeof(INODE) + 12;
        fseek(file, vaiPara, SEEK_SET);
        char tamanhoDiretorio = 0;
        fread(&tamanhoDiretorio, sizeof(char), 1, file);
        tamanhoDiretorio--;
        fseek(file, -1, SEEK_CUR);
        fwrite(&tamanhoDiretorio, sizeof(char), 1, file);

        vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorioAntigo * sizeof(INODE);
        fseek(file, vaiPara, SEEK_SET);
        //
        INODE inodePraRemoverBloco;
        fread(&inodePraRemoverBloco, sizeof(INODE), 1, file);
        //
        int blocosUsados = 0;
        int blocosLivres = contadorDeDiretorio;
        if (inodePraRemoverBloco.NAME[0] != '/')
        {
            blocosLivres--;
        }
        else
            blocosUsados++;
        for (size_t i = 0; i < sizeof(inodePraRemoverBloco.DIRECT_BLOCKS); i++)
        {
            if (inodePraRemoverBloco.DIRECT_BLOCKS[i] == 0)
            {
                blocosLivres--;
            }
            else
                blocosUsados++;
        }

        char apagarBlocosDiretorioAntigo[blocosUsados * tamanhoDoBloco];
        for (size_t i = 0; i < sizeof(apagarBlocosDiretorioAntigo); i++)
        {
            apagarBlocosDiretorioAntigo[i] = 0;
        }

        int indexIncrementar = 0;
        for (size_t i = 0; i < blocosUsados; i++)
        {
            char temp[tamanhoDoBloco];
            vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1 + inodePraRemoverBloco.DIRECT_BLOCKS[i] * tamanhoDoBloco;
            fseek(file, vaiPara, SEEK_SET);
            fread(&temp, tamanhoDoBloco, 1, file);

            for (size_t j = 0; j < tamanhoDoBloco; j++)
            {
                apagarBlocosDiretorioAntigo[j + indexIncrementar] = temp[j];
            }
            indexIncrementar = tamanhoDoBloco;
        }

        for (size_t i = 0; i < sizeof(apagarBlocosDiretorioAntigo); i++)
        {
            if (i + 1 < sizeof(apagarBlocosDiretorioAntigo))
            {
                if (apagarBlocosDiretorioAntigo[i + 1] == 0)
                {
                    indexIncrementar++;
                }
                else
                {
                    if (apagarBlocosDiretorioAntigo[i] == indexDoCaminho)
                        apagarBlocosDiretorioAntigo[i] = apagarBlocosDiretorioAntigo[i + 1];

                    if (i != 0)
                        if (apagarBlocosDiretorioAntigo[i] == apagarBlocosDiretorioAntigo[i - 1])
                            apagarBlocosDiretorioAntigo[i] = apagarBlocosDiretorioAntigo[i + 1];
                }
            }
        }

        indexIncrementar = 0;
        for (size_t i = 0; i < blocosUsados; i++)
        {
            vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1 + inodePraRemoverBloco.DIRECT_BLOCKS[i] * tamanhoDoBloco;
            fseek(file, vaiPara, SEEK_SET);
            char valorTemporario;
            for (size_t j = 0; j < tamanhoDoBloco; j++)
            {
                valorTemporario = apagarBlocosDiretorioAntigo[j + indexIncrementar];
                fwrite(&valorTemporario, sizeof(char), 1, file);
            }
            indexIncrementar = tamanhoDoBloco;
        }

        bool ocupado;

        if (blocosUsados * 2 <= tamanhoDiretorio)
        {
            ocupado = false;
        }
        else
        {
            for (size_t i = tamanhoDiretorio - 1; i < sizeof(inodePraRemoverBloco.DIRECT_BLOCKS); i++)
            {
                inodePraRemoverBloco.DIRECT_BLOCKS[i] = 0;
            }
            vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorioAntigo * sizeof(INODE);
            fseek(file, vaiPara, SEEK_SET);
            fwrite(&inodePraRemoverBloco, sizeof(INODE), 1, file);
        }

        aumentarTamanhoDiretorio(tamanhoMapaDeBit, indexDoDiretorioNovo, &tamanhoDiretorio, fsFileName);

        vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorioNovo * sizeof(INODE);
        fseek(file, vaiPara, SEEK_SET);
        //
        INODE novoInode;
        fread(&novoInode, sizeof(INODE), 1, file);
        //
        int ultimoBlocoUsado = 0;
        for (size_t i = 0; i < sizeof(novoInode.DIRECT_BLOCKS); i++)
        {
            if (novoInode.DIRECT_BLOCKS[i] != 0)
            {
                if (indexDoCaminho != novoInode.DIRECT_BLOCKS[i])
                    ultimoBlocoUsado = i;
                else
                    ocupado = true;
            }
        }
        int tamanhoTemporario = tamanhoDiretorio - 1;
        while (tamanhoTemporario > tamanhoDoBloco)
        {
            tamanhoTemporario -= tamanhoDoBloco;
        }
        int indexBlocoDiretorio = novoInode.DIRECT_BLOCKS[ultimoBlocoUsado] * 2 + tamanhoTemporario;

        if (tamanhoDiretorio <= tamanhoDoBloco)
        {
            ocupado = false;
        }
        else
        {
            vaiPara = 3 + tamanhoMapaDeBit;
            fseek(file, vaiPara, SEEK_SET);
            char indexBlocoLivre = 0;
            for (int i = 0; i < numInodes; i++)
            {
                fread(&inode, sizeof(INODE), 1, file);

                for (size_t j = 0; j < sizeof(inode.DIRECT_BLOCKS); j++)
                {
                    if (inode.DIRECT_BLOCKS[j] <= indexBlocoLivre)
                    {
                    }
                    else
                        indexBlocoLivre = inode.DIRECT_BLOCKS[j];
                }
            }
            //

            indexBlocoLivre++;
            indexBlocoDiretorio = indexBlocoLivre * 2;

            vaiPara = 3 + tamanhoMapaDeBit + indexDoDiretorioNovo * sizeof(INODE);
            fseek(file, vaiPara, SEEK_SET);
            fread(&novoInode, sizeof(INODE), 1, file);
            int contadorNovoInode = 0;
            // index novo bloco
            for (size_t i = 1; i < sizeof(novoInode.DIRECT_BLOCKS); i++)
            {
                if (novoInode.DIRECT_BLOCKS[i] != 0)
                    contadorNovoInode++;
                else
                {
                    novoInode.DIRECT_BLOCKS[i] = indexBlocoLivre;
                    break;
                }
            }
            fseek(file, vaiPara, SEEK_SET);
            fwrite(&novoInode, sizeof(INODE), 1, file);
        }

        vaiPara = 3 + tamanhoMapaDeBit + numInodes * sizeof(INODE) + 1 + indexBlocoDiretorio;
        fseek(file, vaiPara, SEEK_SET);
        fwrite(&indexDoCaminho, sizeof(char), 1, file);

        // alterar o bitmap
        char valorDoBitMap[numDeBlocos];
        for (size_t i = 0; i < sizeof(valorDoBitMap); i++)
        {
            if (i != 0)
                valorDoBitMap[i] = 0;
            else
                valorDoBitMap[i] = 1;
        }
        vaiPara = 3 + tamanhoMapaDeBit;
        fseek(file, vaiPara, SEEK_SET);
        //
        for (int i = 0; i < numInodes; i++)
        {
            fread(&novoInode, sizeof(INODE), 1, file);
            for (size_t j = 0; j < sizeof(novoInode.DIRECT_BLOCKS); j++)
            {
                if (novoInode.DIRECT_BLOCKS[j] != 0)
                {
                    valorDoBitMap[novoInode.DIRECT_BLOCKS[j]] = 1;
                }
            }
        }

        int valorParaHex = 0; 

        for (size_t i = 0; i < sizeof(valorDoBitMap); i++)
        {
            if (valorDoBitMap[i] != 0)
                valorParaHex += pow(2, i);
        }

        fseek(file, 3, SEEK_SET);
        fwrite(&valorParaHex, sizeof(char), 1, file); 
    }
    else
    {
        int nomeSize = sizeof(novoNomePath);
        int vaiPara = 3 + tamanhoMapaDeBit + indexDoCaminho * sizeof(INODE) + 2;
        fseek(file, vaiPara, SEEK_SET);
        for (size_t i = 0; i <nomeSize ; i++)
        {
            char newName = novoNomePath[i];
            int sizeChar =  sizeof(char);
            fwrite(&newName,sizeChar, 1, file);
        }
    }
    fclose(file);
}

