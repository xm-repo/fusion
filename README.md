

[Задача тут](https://github.com/xm-repo/fusion/blob/master/vacancy_server_cpp_task_LOGS_Fusion%20Core.pdf)


```mkdir build && cd build && cmake .. && make```


собирал на cmake 3.16, gcc 9.3.0 Ubuntu 20.04 & MSVC 19.16.27041.0, на младших версиях скорее всего не соберется


запуск ./MegaBI 'папка' 'кол-во потоков'


Код не комментировал, если что, могу расписать. 
Использовал [catch2](https://github.com/catchorg/Catch2) для мини-теста, [date](https://github.com/HowardHinnant/date) и [rapidjson](https://github.com/Tencent/rapidjson).
4кк укороченных записей генерил питончиком, обрабатывается за [несколько секунд на еле-живом ноуте](https://github.com/xm-repo/fusion/blob/master/perf.svg).
