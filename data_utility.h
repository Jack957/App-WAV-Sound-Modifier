#ifndef DATA_UTILITY_H
#define DATA_UTILITY_H
#include "complex.h"
#include <vector>

/**
 * @brief Pomocná struktura pro výpočty.
 */
struct DataUtility
{
    /**
     * @brief   Nalezne následující mocninu dvojky.
     * @param x Unsigned číslo.
     * @return  Vrací následující mocninu dvojky za číslem x.
     */
    static size_t findNextTo2Exp(size_t x);

    /**
     * @brief                   Zkonvertuje Bajtové číslo na komplexní.
     * @param buffer            Vstup z kterého se čte.
     * @param size_of_sample    Velikost čísla, které se čte.
     * @return                  Vrací komplexní číslo zpracované ze vstupu.
     */
    static complex fromCharsToComplex(char *buffer, size_t size_of_sample);

    /**
     * @brief                   Zkonvertuje komplexní číslo na Bajtové.
     * @param number            Vstupní komplexní číslo.
     * @param size_of_sample    Velikost čísla, které se zapíše.
     * @return                  Vrací vektor charů jako bitový zápis x v Little Endian.
     */
    static std::vector<char> fromComplexToChars(complex number, size_t size_of_sample);

    /**
     * @brief                       Přescaluje na rozmezí [-1,+1], respektive zpět.
     * @param[in,out] number        Číslo ke scalování.
     * @param[in] size_of_sample    Velikost čísla, z kterého se převádí.
     * @param[in] inverse           Scaluje se dopredu nebo dozadu.
     *
     *  Převádí reálnou část komplexního čísla na rozsah [-1,+1] respektive na velikost
     *  čísla vstupu, což je [0,255] nebo [-32768,+32767].
     */
    static void scaleComplex(complex &number, size_t size_of_sample, bool inverse);

    /**
     * @brief           Načte preset.
     * @param filename  Jméno souboru presetu.
     * @return          Vrátí data presetu ve vectoru.
     */
    static std::vector<double> loadPreset(const char* filename);
};

#endif // DATA_UTILITY_H
