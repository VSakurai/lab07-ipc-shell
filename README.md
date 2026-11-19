# Lab 7: Pipes, Redirection, Signals and Mini-Shell

**Студент:** Галушка В.Ю.
**Предмет:** Архітектура ЕОМ, операційні системи та системне програмування

---

## Зміст

- [Part I. Pipes](#part-i-pipes)
- [Part II. Redirection](#part-ii-redirection)
- [Part III. Signals](#part-iii-signals)
- [Part IV. Mini-Shell](#part-iv-mini-shell)
- [Висновок](#висновок)

---

## Part I. Pipes

### 1. Pipe Basic (`pipe_basic.c`)

**Результат:**

```text
Read end:  3
Write end: 4
Received: Hello through pipe!
```

**Відповіді:**

1. _Чи pipe є regular file?_ — Ні, це анонімний канал (IPC).
2. _Чому до pipe можна застосовувати `read()` і `write()`?_ — Тому що pipe працює через file descriptors.
3. _Чому `pipe()` повертає два file descriptors?_ — Один для читання, інший для запису.

### 2. Pipe між Processes (`pipe_processes.c`)

**Descriptor map:**

```text
Parent                         Child
fd 3 → Pipe Read               fd 3 → Pipe Read
fd 4 → Pipe Write              fd 4 → Pipe Write
```

**Після закриття:**

- Parent: `close(pipefd[0])` — закриває read end.
- Child: `close(pipefd[1])` — закриває write end.

### 3. Pipe без `close()` (`pipe_no_close.c`)

**Результат:** Програма зависає, оскільки child чекає на EOF, але write end залишається відкритим у parent.

**Висновок:** Для EOF потрібно, щоб усі write ends були закриті.

### 4. strace для `pipe_processes`

```text
pipe([3, 4]) = 0
clone(...) = 12345
close(3) = 0
write(4, "Message from parent\n", 20) = 20
close(4) = 0
read(3, "Message from parent\n", 127) = 20
close(3) = 0
wait4(12345, ...) = 12345
```

---

## Part II. Redirection

### 5. `dup2()` (`dup2_output.c`)

**Результат:** Файл `redirected.txt` містить `This goes to the file`.

**Descriptor map:**

- До: `fd 1 → Terminal`, `fd 3 → redirected.txt`
- Після `dup2`: `fd 1 → redirected.txt`, `fd 3 → redirected.txt`
- Після `close(fd)`: `fd 1 → redirected.txt`

### 6. `exec()` + Redirection (`redirect_exec.c`)

**Результат:** Файл `ls-output.txt` містить вивід `ls -l`.

### 7. Pipeline (`pipeline.c`) — `ls -l | wc -l`

**Descriptor map:**

```text
Child 1: 1 → pipe write
Child 2: 0 → pipe read
Parent: closes both ends
```

**Результат:** Виводить кількість файлів у поточній директорії.

---

## Part III. Signals

### 8. SIGINT (`signal_int.c`)

**Результат:** При натисканні Ctrl+C виводиться `SIGINT received`.

**Чому `write()`, а не `printf()`?** — `write()` є async-signal-safe, `printf()` — ні.

### 9. SIGTERM (`signal_parent_child.c`)

**Результат:**

```text
Parent: sending SIGTERM to child 12345
Child: SIGTERM received
Parent: child terminated
```

**Різниця між SIGTERM та SIGKILL:**

- `SIGTERM` — можна перехопити, програма може завершитися коректно.
- `SIGKILL` — не можна перехопити, програма вбивається негайно.

**Чому SIGKILL не можна catch?** — Це примусове завершення на рівні ядра.

---

## Part IV. Mini-Shell

### 10. Mini-Shell (`mini_shell.c`)

**Підтримувані команди:**

- `ls`, `ls -l`, `echo hello`, `pwd`
- `cd /tmp` (built-in)
- `echo hello > output.txt` (redirection)
- `ls -l | wc -l` (pipeline)
- `exit`

### 11. Чому `cd` — built-in?

`cd` повинен бути built-in, оскільки він змінює working directory самого shell. Якщо виконати `cd` у child process, зміни відбудуться лише в child, а parent shell залишиться в старій директорії.

### 12. Ctrl+C Test

```text
myshell> sleep 30
^C
myshell> echo alive
alive
```

Shell продовжує працювати, оскільки він ігнорує SIGINT, а child відновлює default behavior.

### 13. Відповіді на контрольні питання

1. **Що таке IPC?** — Inter-Process Communication (міжпроцесна взаємодія).
2. **Що створює `pipe()`?** — Анонімний канал для передачі даних.
3. **Що означає `pipefd[0]`?** — Read end (для читання).
4. **Що означає `pipefd[1]`?** — Write end (для запису).
5. **Чому pipe можна використовувати через `read()` та `write()`?** — Тому що pipe працює через file descriptors.
6. **Які descriptors отримує child після `fork()`?** — Копії всіх fd батька.
7. **Чому потрібно закривати unused pipe ends?** — Щоб уникнути зависання (EOF не настане, поки всі write ends відкриті).
8. **Коли `read()` із pipe повертає EOF?** — Коли всі write ends закриті.
9. **Що може статися, якщо parent залишить write end відкритим?** — Child буде чекати вічно (зависання).
10. **Що робить `dup2()`?** — Дублює file descriptor на вказаний номер.
11. **Як реалізується `>`?** — `open()` + `dup2()` + `close()` + `exec()`.
12. **Чому program не повинна знати, що її stdout redirected?** — Тому що вона пише в fd 1, а kernel вже перенаправив його.
13. **Як побудувати `program1 | program2`?** — `pipe()` + `fork()` + `dup2()` + `exec()`.
14. **Який descriptor першої program підключити до pipe?** — stdout (fd 1).
15. **Який descriptor другої program підключити до pipe?** — stdin (fd 0).
16. **Що таке signal?** — Асинхронне повідомлення процесу.
17. **Чим signal відрізняється від pipe?** — Signal — асинхронний, pipe — синхронний (data flow).
18. **Що робить `kill()`?** — Надсилає signal процесу.
19. **Чим SIGTERM відрізняється від SIGKILL?** — SIGTERM можна перехопити, SIGKILL — ні.
20. **Чому не можна catch SIGKILL?** — Це примусове завершення на рівні ядра.
21. **Що таке SIGINT?** — Signal від Ctrl+C.
22. **Чому `printf()` не є хорошим вибором для signal handler?** — Він не async-signal-safe.
23. **Що таке async-signal-safe function?** — Функція, яку можна безпечно викликати в signal handler.
24. **Чому shell є user-space program?** — Він не є частиною ядра.
25. **Що робить `fork()` у shell?** — Створює child для виконання команди.
26. **Що робить `exec()`?** — Замінює program image на нову програму.
27. **Для чого `waitpid()`?** — Чекає завершення child.
28. **Чому `cd` повинен бути shell built-in?** — Змінює working directory самого shell.
29. **Чому `exit` повинен виконуватися самим shell?** — Завершує сам shell, а не child.
30. **Чому `system()` не підходить?** — Приховує механізми fork/exec/wait.
31. **Що відбувається з fd після `exec()`?** — Залишаються відкритими (якщо не позначені `FD_CLOEXEC`).
32. **Як `fork()`, `pipe()` та `dup2()` створюють pipeline?** — `fork` створює processes, `pipe` з'єднує їх, `dup2` перенаправляє fd.
33. **Чому pipeline processes можуть виконуватися concurrently?** — Вони працюють паралельно, читаючи/пишучи в pipe.
34. **Що відбувається, якщо reader завершився, а writer продовжує писати?** — Writer отримує SIGPIPE.
35. **Які ресурси повинен закрити parent після pipeline?** — Обидва кінці pipe.

---

## Висновок

У ході лабораторної роботи я дослідив, як shell запускає програми, перенаправляє їх input/output та з'єднує processes через pipe. Я навчився створювати анонімні pipes, використовувати `dup2()` для redirection, надсилати та обробляти signals, а також реалізував власний mini-shell, який підтримує зовнішні команди, built-ins (`cd`, `exit`), output redirection та pipeline з двох команд. Я побачив, що `fork`, `exec`, `wait`, `pipe`, `dup2` та signals — це фундаментальні абстракції, які разом утворюють основу роботи shell.
