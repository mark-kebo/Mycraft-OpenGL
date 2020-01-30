# OPENGL Minecraft clone-3D

Use Button "P" for change object type
------------------------------------------------

Na projekt składa się opis projektu oraz aplikacja renderujące scenę 3D stworzona w C++ i OpenGL (GLSL), używając frameworka przedstawionego na zajęciach (z ewentualnymi modyfikacjami). Aplikacja powinna zostać napisana obiektowo z możliwe zminimalizowaną (najlepiej do 0) redundancją kodu.

Scena 3D powinna zawierać (minimalne wymagania):
– co najmniej 4 różne obiekty 3D
– co najmniej 2 dynamiczne światła
– co najmniej 2 rodzaje świateł – otoczenia (ambient) i punktowe (point light) – oświetlenie powinno być zaimplementowane przy użyciu modelu Blinna-Phonga
– część obiektów powinna używać tekstur wczytywanych w postaci obrazka z pliku (format .dds lub .ktx), a część powinna używać tekstur proceduralnych (innych od implementowanych na zajęciach)
– część obiektów 3D powinna zawierać duplikacje wierzchołków (ostre krawędzie), a część nie
– scena nie może być statyczna – gracz/obserwator musi mieć możliwość poruszanuia się po scenie i powinien mieć możliwość wpływu na jej wygląd (np. poruszanie obiektami, światłami itd.)

Mile widziane:
– użycie kilku zestawów shader-ów [1p]
– możliwość zaawansowanej interakcji gracza/obserwatora z wybranymi światłami/obiektami na scenie (np. stworzenie prostej gry) [1.5p]
– „skybox” przy użyciu „cube map” [2p]
– wykorzystanie „instanced rendering”, gdzie dane „per instance” są przesyłane przez UBO [2.5p]
– możliwość renderowania dowolnej liczby swiateł (implementacja „forward rendering”) [3p]
