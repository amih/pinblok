#include "../dappservices/multi_index.hpp"
#include "../dappservices/log.hpp"
#include "../dappservices/oracle.hpp"
#include "../dappservices/cron.hpp"
#include "../dappservices/vaccounts.hpp"
#include "../dappservices/readfn.hpp"
#include "../dappservices/vcpu.hpp"
#include "../dappservices/multi_index.hpp"
// #include "../print.h"

#define DAPPSERVICES_ACTIONS() \
  XSIGNAL_DAPPSERVICE_ACTION \
  IPFS_DAPPSERVICE_ACTIONS \
  VACCOUNTS_DAPPSERVICE_ACTIONS \
  LOG_DAPPSERVICE_ACTIONS \
  CRON_DAPPSERVICE_ACTIONS \
  ORACLE_DAPPSERVICE_ACTIONS \
  VCPU_DAPPSERVICE_ACTIONS \
  READFN_DAPPSERVICE_ACTIONS
#define DAPPSERVICE_ACTIONS_COMMANDS() \
  IPFS_SVC_COMMANDS()ORACLE_SVC_COMMANDS()CRON_SVC_COMMANDS()VACCOUNTS_SVC_COMMANDS()LOG_SVC_COMMANDS()READFN_SVC_COMMANDS()VCPU_SVC_COMMANDS()
#define CONTRACT_NAME() pinblok
using std::string;
CONTRACT_START()


      TABLE stat {
          uint64_t   counter = 0;
      };

    typedef eosio::singleton<"stat"_n, stat> stats_def;
    bool timer_callback(name timer, std::vector<char> payload, uint32_t seconds){

        stats_def statstable(_self, _self.value);
        stat newstats;
        if(!statstable.exists()){
            statstable.set(newstats, _self);
        }
        else{
            newstats = statstable.get();
        }
        auto reschedule = false;
        if(newstats.counter++ < 3){
            reschedule = true;
        }
        statstable.set(newstats, _self);
        return reschedule;
        // reschedule

      }
    [[eosio::action]] void testschedule() {
        std::vector<char> payload;
        schedule_timer(_self, payload, 2);
    }


      struct dummy_action_hello {
          name vaccount;
          uint64_t b;
          uint64_t c;

          EOSLIB_SERIALIZE( dummy_action_hello, (vaccount)(b)(c) )
      };

      [[eosio::action]] void hello(dummy_action_hello payload) {
        require_vaccount(payload.vaccount);

        print("hello from ");
        print(payload.vaccount);
        print(" ");
        print(payload.b + payload.c);
        print("\n");
      }

      [[eosio::action]] void hello2(dummy_action_hello payload) {
        print("hello2(default action) from ");
        print(payload.vaccount);
        print(" ");
        print(payload.b + payload.c);
        print("\n");
      }

    // vAccount (name, balance, homebaseclubname)
    // club (clubname, createdat, managername, streetaddress, city, state, country, openinghours)
    // machine (machinename, createdat, ownername, clubname, serialnumber)
    // payments (autoincrementid, membername, createdat, quantity, approvalstatus)
    // group (groupname, createdat, description, meetingtimes)
    // machinehighscore (machinename, createdat, playername, machinestate, highscore)
    // playerhighscore  (playername, createdat, machinename, machinestate, highscore)

    ////////////////////
    // vAccount
    ////////////////////
    TABLE account {
        //  name username;
         extended_asset balance;
        //  name clubname; // homebase clubname, the default club this user chose
         uint64_t /*name*/ primary_key() const { return balance.contract.value; } // username; }
    };

    typedef dapp::multi_index<"vaccounts"_n, account> cold_accounts_t;
    typedef eosio::multi_index<".vaccounts"_n, account> cold_accounts_t_v_abi;
    TABLE shardbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vaccounts"_n, shardbucket> cold_accounts_t_abi;

    ////////////////////
    // club (clubname, createdat, managername, streetaddress, city, state, country, openinghours)
    ////////////////////
    TABLE club {
        name clubname;
        uint64_t createdat;
        name managername;
        string streetaddress;
        string city;
        string state;
        string country;
        string openinghours;

        uint64_t primary_key() const { return clubname.value; }
    };

    typedef dapp::multi_index<"vclub"_n, club> club_t;
    typedef eosio::multi_index<".vclub"_n, club> club_t_v_abi;
    TABLE shardclubbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vclub"_n, shardclubbucket> club_t_abi;

    [[eosio::action]] void clubupsert( name   p_clubname
                                      , name   p_managername
                                      , string p_streetaddress
                                      , string p_city
                                      , string p_state
                                      , string p_country
                                      , string p_openinghours
                                      ){
        club_t clubtable( _self, _self.value );
        auto club_iterator =  clubtable.find(p_clubname.value);
        if (club_iterator == clubtable.end()) {
            club_iterator  = clubtable.emplace(_self,  [&](auto& new_club) {
                new_club.clubname      = p_clubname;
                new_club.managername   = p_managername;
                new_club.streetaddress = p_streetaddress;
                new_club.city          = p_city;
                new_club.state         = p_state;
                new_club.country       = p_country;
                new_club.openinghours  = p_openinghours;
            });
        } else {// else... modify? (UPSERT?)
            clubtable.modify( club_iterator, /*eosio::same_payer*/_self, [&]( auto& edit_club ) {
                edit_club.managername   = p_managername;
                edit_club.streetaddress = p_streetaddress;
                edit_club.city          = p_city;
                edit_club.state         = p_state;
                edit_club.country       = p_country;
                edit_club.openinghours  = p_openinghours;
            });
        }
    }
    [[eosio::action]] void clubdelete( name   p_clubname ){
        club_t clubtable( _self, _self.value );
        auto club_iterator =  clubtable.find(p_clubname.value);
        if (club_iterator != clubtable.end()) {
            club_iterator  = clubtable.erase(club_iterator);
        }
    }
    // cleos wallet import
    // 5KMJLwnj9MwNDNYuuqBjP1eoVuCuqvHy8D5dhA3WFHpQVseaPyv (for testing only...)
    // cleos -u http://localhost:13015 push action pinblok clubupsert '["ourclub","joshtheman", "Main street 1200 East :)","Tel Aviv","IL","Israel","24x7 except for Shabbath"]' -p pinblok@active
    // cleos -u http://localhost:13015 push action pinblok clubupsert '["winnipegpin","jkjkjk", "1400 Infinity road","Winnipeg","Manitoba","Canada","24 hours Mon - Fri"]' -p pinblok@active
    // cleos -u http://localhost:13015 push action pinblok clubupsert '["bestpinball","joshtheman", "4th floor, 212th, 47th E. 5th Ave","New york","NY","USA","24x7x365"]' -p pinblok@active
    // zeus get-table-row "pinblok" "vclub" "pinblok" "ourclub"     "name" | python -m json.tool
    // zeus get-table-row "pinblok" "vclub" "pinblok" "winnipegpin" "name" | python -m json.tool
    // zeus get-table-row "pinblok" "vclub" "pinblok" "bestpinball" "name" | python -m json.tool

    // {
    //     "row": {
    //         "city": "Tel Aviv",
    //         "clubname": "123a",
    //         "country": "Israel",
    //         "createdat": "0",
    //         "managername": "333",
    //         "openinghours": "24x7",
    //         "state": "IL",
    //         "streetaddress": "streetAddress"
    //     }
    // }

    [[eosio::action]] void withdraw( name to, name token_contract){

        require_auth( to );
        require_recipient( to );
        auto received = sub_all_cold_balance(to, token_contract);
        action(permission_level{_self, "active"_n}, token_contract, "transfer"_n,
            std::make_tuple(_self, to, received, std::string("withdraw")))
        .send();
    }

    void transfer( name from,
                    name to,
                    asset        quantity,
                    string       memo ){
        require_auth(from);
        if(to != _self || from == _self || from == "eosio"_n || from == "eosio.stake"_n || from == to)
            return;
        if(memo == "seed transfer")
            return;
        if (memo.size() > 0){
          name to_act = name(memo.c_str());
          eosio::check(is_account(to_act), "The account name supplied is not valid");
          require_recipient(to_act);
          from = to_act;
        }
        extended_asset received(quantity, get_first_receiver());
        add_cold_balance(from, received);
     }

   private:
      extended_asset sub_all_cold_balance( name owner, name token_contract){
           cold_accounts_t from_acnts( _self, owner.value );
           const auto& from = from_acnts.get( token_contract.value, "no balance object found" );
           auto res = from.balance;
           from_acnts.erase( from );
           return res;
      }

      void add_cold_balance( name owner, extended_asset value){
           cold_accounts_t to_acnts( _self, owner.value );
           auto to = to_acnts.find( value.contract.value );
           if( to == to_acnts.end() ) {
              to_acnts.emplace(_self, [&]( auto& a ){
                a.balance = value;
              });
           } else {
              to_acnts.modify( *to, eosio::same_payer, [&]( auto& a ) {
                a.balance += value;
              });
           }
      }

    VACCOUNTS_APPLY(((dummy_action_hello)(hello))((dummy_action_hello)(hello2)))

CONTRACT_END((clubupsert)(clubdelete)(withdraw)(hello)(hello2)(regaccount)(testschedule))