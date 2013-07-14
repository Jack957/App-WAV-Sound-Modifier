#ifndef WAVE_H
#define WAVE_H
#include "fft.h"
#include <vector>

/**
 * @brief Třída reprezentující WAV soubor.
 *
 * Hlavní třída pro načtení a zpracování WAV souboru.
 */
class Wave
{
public:
    /**
     * @brief Struktura pro uložení RIFF Chunku.
     *
     * Struktura pro uložení takzvaného RIFF Chunku,
     * který říká, jakého formátu je danný soubor.
     * např.: WAV, OGG, atd.
     */
    struct RiffChunk
    {
        char ID[4];
        unsigned int length;
        char Format[4];
    } rchunk;

    /**
     * @brief Strukura pro uložení FMT Chunku.
     *
     * Strukura pro uložení "fmt " chunku.
     * Obsahuje všechny informace o WAV souboru.
     */
    struct FmtChunk
    {
        char ID[4];
        unsigned int length;
        unsigned short int AudioFormat;
        unsigned short int NumChannels;
        unsigned int SampleRate;
        unsigned int ByteRate;
        unsigned short int BlockAlign;
        unsigned short int BitsPerSample;
    } fchunk;

    /**
     * @brief Struktura pro uložení hlavičky DATA Chunku.
     *
     * Slouží pro uložení hlavičky "data" chunku.
     * Obsahuje pouze délku dat.
     */
    struct DataChunkHeader
    {
        char ID[4];
        unsigned int length;
    };

    /**
     * @brief Třída pro uložení DATA Chunku.
     *
     * Zkopíruje si do sebe data z WAV podle délky z Data hlavičky.
     */
    class DataChunk
    {
    public:
        DataChunkHeader head;  /**< Hlavička Data chunku. */
        char *data; /**< Raw data Data chunku. */

        /**
         * @brief       Konstruktor.
         * @param head  Odkaz na hlavičku data chunku.
         * @param data  Data Data chunku.
         */
        inline DataChunk(const DataChunkHeader &head, char *data) : head(head), data(data) {}

        /**
         * @brief       Kopírovací konstruktor.
         * @param other Data chunk ke zkopírování.
         */
        DataChunk(const DataChunk &other) : head(other.head)
        {
            data = new char[head.length];
            std::copy(other.data,other.data+head.length,data);
        }

        /**
         * @brief   Destruktor
         */
        inline ~DataChunk() { delete [] data; }
    } dchunk;

    std::vector<complex> *PData;    /**< Rozparsové data, podle kanálů. */

    /**
     * @brief                       Mění frekvenční složky wavu.
     * @param other                 Vstupní preset.
     * @param loudnessNormalization Udává, jestli se má po skončení Equalizace normalizovat zvuk.
     *
     * Změní frekvenční složky wavu, podle zadaného presetu, eventulně normalizuje hlasitost.
     */
    void equalizeWith(std::vector<double> &other, bool loudnessNormalization = true);

    /**
     * @brief                           Mění hlasitost wavu.
     * @param per                       Číslo, udávající novou hlasitost v procentech.
     * @param loudnessNormalization     Udává, jestli se má po skončení Equalizace normalizovat zvuk.
     *
     * Změní hlasitost celého wavu podle procent zadaných parametrem per, eventulně normalizuje hlasitost.
     */
    void changeVolumeToPercentage(unsigned int per, bool loudnessNormalization = true);

    /**
     * @brief           Uloží wave do WAV souboru.
     * @param filename  Jméno souboru, kam se Wave struktura uloží.
     *
     * Ukládá Wave strukturu do souboru, který je ve formatu WAV podle norem Microsoftu.
     */
    void saveToWaveFile(const char * filename);

    /**
     * @brief           Načte wave z WAV souboru.
     * @param filename  Jméno souboru, z kterého se načte WAV soubor do Wave struktury.
     * @return          Vrací pointer na načtenou Wave strukturu.
     *
     * Slouží k načtení WAV souboru do Wave struktury, včetně rozparsování dat z Data chunku do přehledného formátu.
     */
    static Wave* fromFilename(const char* filename);

private:
    /**
     * @brief           Konstruktor.
     * @param rchunk    Odkaz na RIFF Chunk.
     * @param fchunk    Odkaz na FMT Chunk.
     * @param dchunk    Odkaz na DATA Chunk.
     */
    inline Wave(const RiffChunk& rchunk, const FmtChunk& fchunk, const DataChunk& dchunk): rchunk(rchunk), fchunk(fchunk), dchunk(dchunk) {}

    /**
     * @brief               Vytáhne z PData část dat.
     * @param channel       Číslo kanálu, z kterého se bude číst.
     * @param from          Index odkud data zkopírovat.
     * @param countData     Počet dat, které se mají načíst ze vstupu.
     * @param countForFFT   Počet dat, které mají být na výstupu.
     * @return              Vrací vektor komplexních čísel o velikosti coutForFFT. (Doplní nuly za countData)
     */
    std::vector<complex> getPieceOfChannel(size_t channel, size_t from, size_t countData, size_t countForFFT);

    /**
     * @brief           Přepíše část dat v PData.
     * @param data      Vstup vektor komplexních čísel, který zmení část dat Wavu.
     * @param channel   Číslo kanálu, na kterém se změna provede.
     * @param from      Index od kterého se budou data zapisovat.
     * @param countData Velikost dat, která se přepíšou.
     */
    void setPieceOfChannel(const std::vector<complex> &data, size_t channel, size_t from, size_t countData);

    /**
     * @brief   Aplikuje preset na wave.
     * @param a Vstup vektor komplexních čísel, který je transformovaný FFT funkcí. (Frekvenční spektrum)
     * @param b Vektor čísel, která vyfiltrují dané frekvence ze vstupu.
     * @return  Vrací vyfiltrovaný vektor komplexních čísel, který půjde do Inverzní FFT funkce.
     */
    static std::vector<complex> applyFilter(const std::vector<complex> &a, std::vector<double> &b);

    /**
     * @brief Ztlumí wave, pokud někde přesahuje max. hlasitost.
     *
     * Pomocná funkce, která najde největší reálnou část ze všech samplů (sampl s největší hlasitostí)
     * a zeslabí potom celý wav tak, aby největší sampl měl největší možnou hlasitost z rozsahu vstupního
     * WAV. Čili slouží k normalizaci hlasitosti, tak aby žádná nepřetýkala rozsah vstupního WAV.
     * Neprovede se, pokud žádný sampl nepřetýká.
     */
    void loudnessNormalization();

    /**
     * @brief       Naparsuje WAV soubor.
     * @param in    Vstupní file stream WAV souboru.
     * @return      Vrací naparsovanou strukturu z WAV souboru.
     */
    static Wave* fromFileStream(std::ifstream& in);

    /**
     * @brief Rozparsuje Raw data do PData.
     *
     * Funkce slouží k rozparsování dat z datachunku do vector<complex>[NumChannels],
     * aby se s datama poté lépe pracovalo.
     */
    void ParseData();

    /**
     * @brief   Složí PData zpět na Raw data.
     * @return  Vrací, jestli nenastala chyba.
     *
     * Slouží k opačné funkci co ParseData(), veme daný vector a složí z něj výstup,
     * z kterého se bude skládat výstup.
     */
    bool ComposeData();
};
#endif // WAVE_H
