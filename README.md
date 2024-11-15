# 1. Definiție
Un keylogger se ocupă cu interceptarea evenimentelor unei tastaturi. Acesta este folosit pentru a urmări activitatea utilizatorilor și are diferite scopuri: obținerea accesului la date confidențiale, monitorizarea activității angajaților, capturarea ecranului etc.

# 2. Organizarea proiectului
- **Partea 1**: Dezvoltarea efectivă a modulului kernel care va intercepta semnale de la tastatură.
- **Partea 2**: Gestionarea și stocarea datelor interceptate, precum și transmiterea acestora, dacă este cazul.

# 3. Funcționalități principale
1. **Interceptarea tastelor**: Captarea evenimentelor de la tastatură în modul kernel.
2. **Stocarea tastelor**: Salvarea logurilor într-un fișier local într-un mod ascuns.
3. **Persistența modulului**: Menținerea modulului activ între reboot-uri.
4. **Transmiterea datelor**: Posibilitatea de a trimite logurile către un server extern.
5. **Ascunderea modulului**: Modulul kernel nu este vizibil în lista de module standard (`lsmod`).
6. **Gestionarea erorilor**: Tratarea situațiilor neprevăzute în modul kernel și curățarea memoriei la descărcare.

Direcții viitoare de dezvoltare, cum ar fi criptarea datelor logate sau adăugarea de capabilități suplimentare (ex: capturarea ecranului).

# 4. Partea 1
Dezvoltarea modulului de kernel a fost împărțită în mai multe funcții, precum: `callback`, `init`, `exit`, `fflush`, `keycode_to_string`, `write_to_logfile`.

- **Callback**: un handler pentru semnalele primite de la tastatură.
- **Init**: funcția de inițializare a modulului de kernel.
- **Exit**: funcție care se ocupă de închiderea modulului, precum și curățarea structurii `KEYLOGGER`.
- **Fflush**: curățarea buffer-ului de citire pentru a face loc unei noi citiri. Implementarea este eficientă pentru a preveni umplerea buffer-ului, ceea ce poate duce la imposibilitatea de a mai citi taste.
- **Keycode_to_string**: funcție folosită pentru a prelucra tastele.
- **Write_to_logfile**: scrierea tastelor interceptate în fișierul de log.

Pentru o structură cât mai clară, am construit o structură denumită `KEYLOGGER`. În cadrul acesteia am inclus:
- O structură de tip `notifier_block`, care se ocupă cu interceptarea tastelor.
- O structură `work_struct`, pentru organizarea execuției proceselor.
- O structură `file`.
- Un `char* buffer`, în care se citesc tastele.
- Un `char* write_buffer`, care preia tastele citite din buffer și le scrie în fișierul de log.

# 5. Identificare de patternuri
Implementarea identificării de patternuri pentru date sensibile precum parole, numere de card, etc.
