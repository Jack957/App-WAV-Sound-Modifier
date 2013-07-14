WAV-Sound-Modifier
========
CZECH version<br>

Bio Equalizer je jednoduchá konzolová aplikace, sloužící k úpravě WAV souborů.<br />
Může sloužit také jako knihovna pro další aplikace.

Co program umí:
- měnit hlasitost
- měnit frekvenční spektrum, dle daného presetu.

Co je to preset a jak vypadá:
- Preset je soubor obsahující modifikující koeficienty, kterými se změní frekvenční spektrum.
- Modifikující koeficienty jsou typu double a jsou odděleny mezerou.
- Na počtu koeficientu nezáleží, ale representují přibližně frekvence od 1 Hz až do \#koeficientů Hz.

Jak program ovládat?
Buď je možno program spustit a navigovat se přes nabídky nebo ho ovládat pomocí parametrů.

Ovládání přes paramety:

zapoctak.exe -i Vstupni_soubor -o Vystupni_soubor [-v Procentuelni_zmena] [-e Preset]

parametry:<br />
-i  Vstupni_soubor - Cesta k WAV souboru, který se bude měnit.<br />
-o  Vystupni_soubor - Cesta k výstupnímu souboru, kam se vstupní WAV uloží.<br />
-v  Procentuelni_zmena - Číslo v procentech, jak se zvuk zeslabí/zesílí.<br />
-e  Preset - Cesta k presetu, který modifikuje frekvenční spektrum vstupního WAVu.<br />


Jak program funguje:
- Nejříve si program načte celý WAV soubor do paměti a trošku si ho předspracuje, aby se s ním lépe pracovalo.
- Změna hlasitosti je primitivní vynásobení každého samplu nějakým skalárem a následné oříznutí přetečení.
- Změna frekvenčního spektra je trošku komplikovanější.

Změna frekvenčního spektra:
- WAV soubor jako takový je nějaká křivka v doméně času. Tato křivka se dá transformovat na doménu frekvence,
pomocí Furierové transformace. Výstup z FFT je pole komplexních čísel. Abslutní hodnota i-tého indexu tohoto pole
je amplituda i-té frekvence. I-tá frekvence má hodnotu i*SampleRate/N, kde SampleRate je daný WAV souborem a N je
číslo o něco větší než SampleRate a je mocninou 2.
- Změnu spektra musíme provádět pro každý kanál zvlášt, protože zvuky z různých kanálů na sobě nemusí nijak
záviset.
- Změna spektra probíhá takto. Pro daný kanál si vezmeme 1. blok o velikost SampleRate. Tento blok natáhneme
nulami do počtu mocniny dvojky, který je potřebný pro korektní zpracování Furierovou transformací. Pošleme tyto
data do dopředné FFT(Fast Furier Transformation) a tím získáme spektrum pro daný blok dat. Toto spektrum
přenásobíme presetem a pošleme ho zpět do zpětné FFT. Toto provedeme se všemi bloky ve všech kanálech. Nakonec
výstupní wave jestě zeslabíme pomocí největšího samplu z dat. Kdybychom toto neudělali, pak by hrozilo ohromné
ořezání výstupního wave a mohli bychom ztratit mnoho dat.
