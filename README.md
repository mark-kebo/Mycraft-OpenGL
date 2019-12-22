# dv_project

Na projekt składa się opis projektu oraz aplikacja renderujące scenę 3D stworzona w C++ i OpenGL (GLSL), używając frameworka przedstawionego na zajęciach (z ewentualnymi modyfikacjami). Aplikacja powinna zostać napisana obiektowo z możliwe zminimalizowaną (najlepiej do 0) redundancją kodu.

Osoby chcące oddawać projekt powinny wcześniej zadeklarować jak będzie wyglądał, konsultując z prowadzącym i wysyłając opis (najpóźniej do 16.01.2019) na mail: dydaktykafais@outlook.com

Scena 3D powinna zawierać (minimalne wymagania):

– co najmniej 4 różne obiekty 3D

– co najmniej 2 dynamiczne światła

– co najmniej 2 rodzaje świateł – otoczenia (ambient) i punktowe (point light) – oświetlenie powinno być zaimplementowane przy użyciu modelu Blinna-Phonga

– część obiektów powinna używać tekstur wczytywanych w postaci obrazka z pliku (format .dds lub .ktx), a część powinna używać tekstur proceduralnych (innych od implementowanych na zajęciach)

– część obiektów 3D powinna zawierać duplikacje wierzchołków (ostre krawędzie), a część nie

– scena nie może być statyczna – gracz/obserwator musi mieć możliwość poruszanuia się po scenie i powinien mieć możliwość wpływu na jej wygląd (np. poruszanie obiektami, światłami itd.)

Mile widziane:

– użycie kilku zestawów shader-ów [1p]

– obliczenia związane z oświetleniem realizowane w przestrzeni widoku [1p]

– poprawna obsługa korekcji gamma [1p]

– część modeli wczytywana z plików (np. obsługa formatu .obj) [1p]

– 4 rodzaje świateł – otoczenia (ambient), punktowe (point light), kierunkowe (directional light) i stożkowe (spot light) [1.5p]
– „normal map” dla obiektów [1.5p]

– „specular intensity map” i „specular power map” (z możliwością ustawienia wartości min i max) dla obiektów [1.5p]

– możliwość zaawansowanej interakcji gracza/obserwatora z wybranymi światłami/obiektami na scenie (np. stworzenie prostej gry) [1.5p]

– automatyczne wyznaczanie „bounding box”, lub „bounding sphere” dla obiektów 3D i „frustum culling” [1.5p]

– „skybox” przy użyciu „cube map” [2p]

– „postprocessing” (np. zastosowanie pełnoekranowego filtra) wykorzystując FBO [2p]

– wykorzystanie „instanced rendering”, gdzie dane „per instance” są przesyłane przez UBO [2.5p]

– możliwość renderowania dowolnej liczby swiateł (implementacja „forward rendering”) [3p]

– zastosowanie PBR zamiast modelu Blinna-Phonga [3p]

Na ocenę projektu oprócz wymienionych powyżej punktów będą miały wpływ takie elementy jak m. in.: design i jakość kodu (zarówno po stronie CPU i GPU), zastosowane optymalizacje, brak wycieków pamięci itd.

Za projekt spełniający tylko minimalne wymagania można uzyskać maksymalnie 6p. Za projekt nie można uzyskać więcej niż 12p.

Projekty należy osobiście przedstawić i przesłać na mail: dydaktykafais@outlook.com najpóźniej do 05.02.2019r 
