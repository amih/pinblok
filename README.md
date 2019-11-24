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

# Backend, one smart contract with some tables and actions

PinBlok demostrates using vAccounts and vRAM to manage a real-world use case including:

1. members
2. clubs (brick & mortar)
3. groups
4. pinball machines
5. high score logs ordered by users and by machines, using 1 to 1 linked vRAM tables.
6. membership payments for each club

The main file I worked on during the hackathon is the [smart contract back end](https://github.com/amih/pinblok/blob/master/dapp/contracts/eos/pinblok/main.cpp)

# Front-end

Front end will comprise a react app with simple login (vAccounts) and each user, according to his permission level will have access to the relevant parts of the database.

# Pinball machine integration

Each pinball machine will have its own QR code mapped web page which will show the machine's information including the type, manufacture year, current owner, and the list of the games played and who were the players and what was their score.

The target of the PINBLOK project is to demonstrate that it is possible to have a regular business database on top of the EOS main net with the help of the liquidApps 2nd layer.

vRAM is a better solution in our minds to the history API problem. Instead of an expensive general purpose history API to query the entire EOS main net, each dapp developer provisions DAPP tokens to several DSP providers and gets to manage their own slice of the history in a seamless integrated vRAM collection of multi-index tables that look and behave like normal RAM multi-index tables to the dapp developer.

# Zeus on GitPod info

Start the Zeus IDE with [Gitpod](https://gitpod.io/#https://github.com/liquidapps-io/zeus-ide)

After your enviroment finishes loading in the editor, you can try:
```bash
zeus create contract mycontract
# edit your contract under dapp/contracts/eos/mycontract
zeus test
```
