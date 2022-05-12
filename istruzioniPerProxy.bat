git config --global http.proxy http://avolta:Physics1907@172.16.1.25:8080
Spiegazione
git config --global http.proxy http://user:pwd@proxy:8080


per sapere il proxy: netstat -a

il proxy è quello che dopo ha il suffisso :8080


git config http.sslVerify false  in caso faccia casino il proxy provare questo.


ultimo proxy:

git config --global http.proxy http://avolta:Physics2209@proxy:8080

