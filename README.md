```            
 ____ ___ _   _ ____  _     ___  _  __
|  _ |_ _| \ | | __ )| |   / _ \| |/ /
| |_) | ||  \| |  _ \| |  | | | | ' / 
|  __/| || |\  | |_) | |__| |_| | . \ 
|_|  |___|_| \_|____/|_____\___/|_|\_\
                                                  
```

# Use vAccounts and vRAM to create an RDBMS without worrying about resources

The DAPP network enables the creation of a production class Database system using vRAM tables
with CRUD methods and relational integrity similar to an SQL Database on top of a high performant blockchain.

PinBlok demostrates using vAccounts and vRAM to manage a real-world use case including:

1. members
2. clubs (brick & mortar)
3. groups
4. pinball machines
5. high score logs ordered by users and by machines, using 1 to 1 linked vRAM tables.
6. membership payments for each club


# Zeus on GitPod info

Start the Zeus IDE with [Gitpod](https://gitpod.io/#https://github.com/liquidapps-io/zeus-ide)

After your enviroment finishes loading in the editor, you can try:
```bash
zeus create contract mycontract
# edit your contract under dapp/contracts/eos/mycontract
zeus test
```
