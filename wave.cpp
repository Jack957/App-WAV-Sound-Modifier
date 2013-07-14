#include "data_utility.h"
#include "wave.h"
#include <fstream>
#include <cassert>
#include <cmath>

Wave* Wave::fromFilename(const char *filename)
{
    std::ifstream in;
    in.open(filename, std::ios_base::in | std::ios_base::binary);
    Wave *ret = fromFileStream(in);
    ret->ParseData();
    in.close();
    return ret;
}

Wave* Wave::fromFileStream(std::ifstream& in)
{
    /* Pokud je file stream otevřen */
    if (in.is_open())
    {
        RiffChunk rch;
        FmtChunk fch;
        DataChunkHeader dchh;

        /* Tak se pokusí načíst RIFF chunk */
        in.read(reinterpret_cast<char*>(&rch),sizeof(RiffChunk));
        std::string a(rch.ID,4);
        /* Pokud není ID RIFF, pak konec s chybou */
        if(a != "RIFF")
        {
            std::cerr << "Nespravny format: " << a << std::endl;
            return 0;
        }

        /* Pokusí se načíst FMT chunk */
        in.read(reinterpret_cast<char*>(&fch),sizeof(FmtChunk));
        a = std::string(fch.ID,4);
        /* Pokud není ID FMT, pak konec s chybou */
        if(a != "fmt ")
        {
            std::cerr << "Nespravny format: " << a << std::endl;
            return 0;
        }

        /* Pokusí se načíst DATA chunk */
        in.read(reinterpret_cast<char*>(&dchh),sizeof(DataChunkHeader));
        a = std::string(dchh.ID,4);
        /* Pokud není ID DATA, pak konec s chybou */
        if(a != "data")
        {
            std::cerr << "Nespravny format: " << a << std::endl;
            return 0;
        }

        /* Zjištění délky do konce souboru od aktuální pozice čtecí hlavy */
        int begin = in.tellg();
        in.seekg (0, std::ios::end);
        int end = in.tellg();
        unsigned int length = end - begin;
        in.seekg(begin);

        char *data;

        /* Kontrola načtené délky z data chunku s délkou do konce souboru, pokud nesouhlasí, tak se opraví */
        if(length != dchh.length)
        {
            std::cerr << "ERROR: Prenastavuju length datachunku na zarovnanou delku." << std::endl;
            std::cerr << "ERROR: Delka podle datachunku - " << dchh.length << std::endl;
            std::cerr << "ERROR: Delka do konce souboru - " << length << std::endl;
            dchh.length = length + length%fch.BlockAlign;
            std::cerr << "ERROR: Delka do konce + zarovnani - " << dchh.length << std::endl;
            /* Inicializace pole dat podle správné délky */
            data = new char[length]();
            /* Zarovnání dat na požadovanou délku */
            for(size_t i = length; i != dchh.length; ++i)
                data[i] = '\0';
        }
        else
        {
            /* Inicializace pole dat podle správné délky */
            data = new char[dchh.length]();
        }

        /* Toto by nemělo nikdy nastat :D, ale co kdyby */
        if(!in.read(data,dchh.length))
            std::cerr << "ERROR: Reading data." << std::endl;

        /* Inicializace data chunku */
        DataChunk dch(dchh,data);
        /* Inicializace a vrácení wave struktury */
        return new Wave(rch,fch,dch);
    }
    else
        return 0;
}

void Wave::saveToWaveFile(const char * filename)
{
    /* Uložení naparsovaných dat do výstupního pole s kontrolou chyb */
    if(!this->ComposeData())
    {
        std::cerr << "ERROR: Neukladam, nastala chyba." << std::endl;
        return;
    }

    /* Vytvoření streamu, kontrola velikostí chunků a následné uložení chunků v pořadí:
        RIFF chunk, FMT Chunk, DATA chunk, DATA a nasledne zavření streamu */
    std::ofstream out(filename, std::ios_base::out | std::ios_base::binary);
    assert(sizeof(RiffChunk) == 12 && sizeof(FmtChunk) == 24 && sizeof(DataChunkHeader) == 8);
    out.write(reinterpret_cast<char*>(&rchunk),sizeof(RiffChunk));
    out.write(reinterpret_cast<char*>(&fchunk),sizeof(FmtChunk));
    out.write(reinterpret_cast<char*>(&dchunk.head),sizeof(DataChunkHeader));
    out.write(dchunk.data,dchunk.head.length);
    out.close();
}

void Wave::changeVolumeToPercentage(unsigned int per, const bool loudnessNormalization)
{
    /* Načtení důležitých proměnných, abych pro ně furt nemusel lézt v cyklech */
    size_t NumChannels = this->fchunk.NumChannels;
    size_t SizeOfSample = this->fchunk.BitsPerSample / 8;
    size_t NumberOfSamples = ( this->dchunk.head.length / NumChannels ) / SizeOfSample;

    /* Pro každý kanál */
    for(size_t i = 0; i != NumberOfSamples; ++i)
        /* A pro každý sampl v kanálu */
        for(size_t j = 0; j != NumChannels; ++j)
                /* Změním hlasitost podle procent per */
                this->PData[j][i] =  this->PData[j][i] * per / 100;

    /* Pokud chceme opravit hlasitost, tak ji opravíme. Defaultně ji opravujem. */
    if(loudnessNormalization)
        this->loudnessNormalization();
}

void Wave::ParseData()
{
    /* Načtení důležitých proměnných, abych pro ně furt nemusel lézt v cyklech */
    size_t NumChannels = this->fchunk.NumChannels;
    size_t SizeOfSample = this->fchunk.BitsPerSample / 8;
    size_t NumberOfSamples = ( this->dchunk.head.length / NumChannels ) / SizeOfSample;
    this->PData = new std::vector<complex>[NumChannels];

    size_t k = 0;
    /* Pro každý sampl */
    for(size_t i = 0; i != NumberOfSamples; ++i)
        /* A pro každý kanál */
        for(size_t j = 0; j != NumChannels; ++j)
        {
            /* Zjistím hodnotu dat, nascaluju ji a uložím do PDat */
            complex tmp = DataUtility::fromCharsToComplex(this->dchunk.data + k,SizeOfSample);
            DataUtility::scaleComplex(tmp,SizeOfSample,false);
            this->PData[j].push_back(tmp);
            k += SizeOfSample;
        }
}

bool Wave::ComposeData()
{
    /* Načtení důležitých proměnných, abych pro ně furt nemusel lézt v cyklech */
    size_t NumChannels = this->fchunk.NumChannels;
    size_t SizeOfSample = this->fchunk.BitsPerSample / 8;
    size_t NumberOfSamples = ( this->dchunk.head.length / NumChannels ) / SizeOfSample;
    if(NumChannels*this->PData[0].size()*SizeOfSample != this->dchunk.head.length)
    {
        std::cerr << "ERROR: Nelze composovat data, protoze upravena jsou jinak dlouha." << std::endl;
        return false;
    }

    size_t k = 0;
    /* Pro každý sampl */
    for(size_t i = 0; i != NumberOfSamples; ++i)
        /* A pro každý kanál */
        for(size_t j = 0; j != NumChannels; ++j)
        {
            /* Nactu komplexni cislo z Pdat, prescaluju zpet, zjistim a ulozim do dat */
            complex tmp = this->PData[j][i];
            DataUtility::scaleComplex(tmp,SizeOfSample,true);
            std::vector<char> ret = DataUtility::fromComplexToChars(tmp, SizeOfSample);
            for(size_t l = 0; l != ret.size(); ++l)
                this->dchunk.data[k+l] = ret[l];
            k += SizeOfSample;
        }
    return true;
}

std::vector<complex> Wave::getPieceOfChannel(size_t channel, size_t from, size_t countData, size_t countForFFT)
{
    if(from+countData >= this->PData[channel].size())
    {
        std::cerr << "Nekonzistence poctu dat ke zkopirovani v dataToComplexPiece." << std::endl;
        return std::vector<complex>();
    }
    std::vector<complex> forReturn;
    /* Jednoduché zkopírování dat z PDat od indexu from do indexu from+countData
     * s následným přidáním nul do délky countForFFT*/
    std::copy(this->PData[channel].begin()+from,this->PData[channel].begin()+from+countData,std::back_inserter(forReturn));
    for(size_t i = from + countData; i != from + countForFFT; ++i)
            forReturn.push_back(complex(0));
    return forReturn;
}

void Wave::setPieceOfChannel(const std::vector<complex> &data, size_t channel, size_t from, size_t countData)
{
    if(from + countData >= this->PData[channel].size())
        std::cerr << "Nekonzistence poctu dat v dataToVectorChar." << std::endl;
    /* Přenastavení PDat z equalizovaných dat ze vstupu prostým přepsáním */
    std::copy(data.begin(),data.begin()+countData,this->PData[channel].begin()+from);
}

void Wave::equalizeWith(std::vector<double> &other, bool loudnessNormalization)
{
    /* Pro každý kanál */
    for(size_t ch = 0; ch < this->fchunk.NumChannels; ++ch)
    {
        size_t i = 0;
        size_t size_of_samples = this->PData[ch].size() - 1;
        size_t count_of_Data = 0, count_for_FFT = 0;
        size_t SampleRate = this->fchunk.SampleRate;
        /* Pro každý sampl z kanálu */
        while(i < size_of_samples)
        {
            /* Nastavím počáteční počty */
            count_of_Data = i + SampleRate > size_of_samples ? size_of_samples-i : SampleRate; //Pojistka, ze nebudu zpracovavat vic dat nez existuje v channelu
            count_for_FFT = DataUtility::findNextTo2Exp(SampleRate);
            /* Načtu blok dat ke zpracování */
            std::vector<complex> InputData = getPieceOfChannel(ch,i,count_of_Data,count_for_FFT);
            /* Pošlu je do Forward FFT */
            CFFT::Forward(&InputData[0],count_for_FFT);
            /* Použiju na ně filtr */
            std::vector<complex> InverseInput = applyFilter(InputData,other);
            /* Pošlu je do Inverze FFT */
            CFFT::Inverse(&InverseInput[0],count_for_FFT,true);
            /* Změním PData na daném rozmezí equalizovanými daty */
            setPieceOfChannel(InverseInput,ch,i,count_of_Data);
            /* Posunu se na další blok dat */
            i += count_of_Data;
        }
    }

    /* Pokud chceme opravit hlasitost, tak ji opravíme. Defaultně ji opravujem. */
    if(loudnessNormalization)
        this->loudnessNormalization();
}

std::vector<complex> Wave::applyFilter(const std::vector<complex> &a, std::vector<double> &b)
{
    std::vector<complex> ret;

    size_t max_freq = floor(a.size()/2);
    /* Doplnění presetu, pokud je moc krátkej. Doplňuji neměnícími frekvencemi. */
    for(size_t i = b.size(); i != max_freq; ++i)
        b.push_back(1);

    /* Aplikování filtru na první polovinu spektra */
    for(size_t i = 0; i < max_freq; ++i)
        ret.push_back(a.at(i)*b.at(i));
    /* Aplikování filtru na druhou polovinu spektra, která je zrcadlová k první. Proto aplikace probíha odzadu */
    for(size_t i = max_freq,j = max_freq - 1; i < a.size() && j >= 0; ++i, --j)
        ret.push_back(a.at(i)*b.at(j));
    return ret;
}

void Wave::loudnessNormalization()
{
    /* Zjistím nejhlasitější sampl */
    complex loudestSample = this->PData[0][0];
    for(size_t i = 0; i != this->fchunk.NumChannels; ++i)
        for(size_t j = 0; j != this->PData[i].size(); ++j)
            if(this->PData[i][j].re() > loudestSample.re())
                loudestSample = this->PData[i][j];

    /* Pokud je hlasitější než maximum, tak celý Wave zeslabím tak,
     * aby tento nehlasitější sampl byl strop rozsahu WAV souboru*/
    if(loudestSample.re() > 1)
    {
        unsigned int zeslabeni = 100 / loudestSample.re();
        this->changeVolumeToPercentage(zeslabeni,false);
    }
}
