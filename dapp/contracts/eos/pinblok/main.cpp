#include "../dappservices/multi_index.hpp"
#include "../dappservices/log.hpp"
#include "../dappservices/oracle.hpp"
#include "../dappservices/cron.hpp"
#include "../dappservices/vaccounts.hpp"
#include "../dappservices/readfn.hpp"
#include "../dappservices/vcpu.hpp"
#include "../dappservices/multi_index.hpp"
#include <eosio/system.hpp>

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

    time_point_sec current_time_point_sec() { return time_point_sec(current_time_point()); }
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
    // payment (autoincrementid, membername, createdat, quantity, approvalstatus)
    // group (groupname, createdat, description, meetingtimes)
    // hiscmach (machinename, createdat, playername,  machinestate, highscore)
    // hiscuser (playername,  createdat, machinename, machinestate, highscore)

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
        time_point_sec createdat;
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

    [[eosio::action]] void clubupsert(  name   p_clubname
                                      , name   p_managername
                                      , string p_streetaddress
                                      , string p_city
                                      , string p_state
                                      , string p_country
                                      , string p_openinghours
                                      ){
        // example integrity checks, protect the contract from flood attacks and keep real RAM usage at a reasonable level.
        // need to verify foreign key, member - manager
        check(p_streetaddress.length() < 170, "streetaddress longer than maximum allowed (170)");
        check(p_city         .length() < 100, "city longer than maximum allowed (100)");
        check(p_state        .length() < 100, "state longer than maximum allowed (100)");
        check(p_country      .length() < 100, "country longer than maximum allowed (100)");
        check(p_openinghours .length() < 100, "openinghours longer than maximum allowed (100)");
        club_t clubtable( _self, _self.value );
        auto club_iterator =  clubtable.find(p_clubname.value);
        if (club_iterator == clubtable.end()) {
            club_iterator  = clubtable.emplace(_self,  [&](auto& new_club) {
                new_club.clubname      = p_clubname;
                new_club.createdat     = current_time_point_sec();
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
    // testing that the smart contract protects from strings being too long:
    // cleos -u http://localhost:13015  push action pinblok clubupsert '["myclub","joshtheman", "street address","city56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890zzzzzzzzzzzzzxxxxxxxxxxxxxxxxxxxccccccccccccccccc", "state", "country", "24x7"]' -p pinblok@active
    // Error 3050003: eosio_assert_message assertion failure
    // Error Details:
    // assertion failure with message: city longer than maximum allowed (100)
    // pending console output: 
    [[eosio::action]] void clubdelete( name   p_clubname ){
        club_t clubtable( _self, _self.value );
        auto club_iterator =  clubtable.find(p_clubname.value);
        if (club_iterator != clubtable.end()) {
            club_iterator  = clubtable.erase(club_iterator);
        }
    }
    ////////////////////
    // machine (machinename, createdat, ownername, clubname, serialnumber)
    ////////////////////
    TABLE machine {
        name machinename;
        time_point_sec createdat;
        name ownername;
        name clubname;
        string serialnumber;

        uint64_t primary_key() const { return machinename.value; }
    };

    typedef dapp::multi_index<"vmachine"_n, machine> machine_t;
    typedef eosio::multi_index<".vmachine"_n, machine> machine_t_v_abi;
    TABLE shardmachinebucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vmachine"_n, shardmachinebucket> machine_t_abi;

    [[eosio::action]] void machineupser( name   p_machinename
                                       , name   p_ownername
                                       , name   p_clubname
                                       , string p_serialnumber
                                      ){
        // TODO: need to verify that the owner exists in the vAccounts table as a valid user. Also, check the club name in the vclub table.
        // need to verify foreign key, member, club...
        machine_t machinetable( _self, _self.value );
        auto machine_iterator =  machinetable.find(p_machinename.value);
        if (machine_iterator == machinetable.end()) {
            machine_iterator  = machinetable.emplace(_self,  [&](auto& new_machine) {
                new_machine.machinename = p_machinename;
                new_machine.createdat   = current_time_point_sec();
                new_machine.ownername   = p_ownername;
                new_machine.clubname    = p_clubname;
                new_machine.serialnumber= p_serialnumber;
            });
        } else {// else... modify? (UPSERT?)
            machinetable.modify( machine_iterator, /*eosio::same_payer*/_self, [&]( auto& edit_machine ) {
                edit_machine.machinename = p_machinename;
                edit_machine.ownername   = p_ownername;
                edit_machine.clubname    = p_clubname;
                edit_machine.serialnumber= p_serialnumber;
            });
        }
    }
    [[eosio::action]] void machinedelet( name   p_machinename ){
        machine_t machinetable( _self, _self.value );
        auto machine_iterator =  machinetable.find(p_machinename.value);
        if (machine_iterator !=  machinetable.end()) {
            machine_iterator  =  machinetable.erase(machine_iterator);
        }
    }
    ////////////////////
    // payment (autoincrementid, membername, createdat, quantity, approvalstatus)
    ////////////////////
    TABLE payment {
        uint64_t autoincrementid;
        time_point_sec createdat;
        name membername;
        asset quantity;
        bool approvalstatus;

        uint64_t primary_key() const { return autoincrementid; }
    };

    typedef dapp::multi_index<"vpayment"_n, payment> payment_t;
    typedef eosio::multi_index<".vpayment"_n, payment> payment_t_v_abi;
    TABLE shardpaymentbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vpayment"_n, shardpaymentbucket> payment_t_abi;

    [[eosio::action]] void paymentinser( name   p_membername
                                       , asset  p_quantity
                                       , bool   p_approvalstatus
                                      ){
        payment_t paymenttable( _self, _self.value );
        // TODO: will be called from payment notification to the business account on the main net.
        // check contract+asset type and accept only from predetermined list.
        // Approval status will be managed by admin / business owner.
        //
        // need to verify foreign key, member
        auto payment_iterator = paymenttable.emplace(_self,  [&](auto& new_payment) {
            new_payment.autoincrementid = paymenttable.available_primary_key();
            new_payment.createdat       = current_time_point_sec();
            new_payment.membername      = p_membername;
            new_payment.quantity        = p_quantity;
            new_payment.approvalstatus  = p_approvalstatus;
        });
    }
    [[eosio::action]] void paymentupdat( uint64_t p_id
                                       , name   p_membername
                                       , asset  p_quantity
                                       , bool   p_approvalstatus
                                      ){
        payment_t paymenttable( _self, _self.value );
        auto payment_iterator =  paymenttable.find(p_id);
        check(payment_iterator == paymenttable.end(), "payment not found!");
        paymenttable.modify( payment_iterator, /*eosio::same_payer*/_self, [&]( auto& edit_payment ) {
            edit_payment.membername     = p_membername;
            edit_payment.quantity       = p_quantity;
            edit_payment.approvalstatus = p_approvalstatus;
        });
    }
    [[eosio::action]] void paymentdelet( uint64_t p_id ){
        payment_t paymenttable( _self, _self.value );
        auto payment_iterator =  paymenttable.find(p_id);
        if (payment_iterator !=  paymenttable.end()) {
            payment_iterator  =  paymenttable.erase(payment_iterator);
        }
    }

    ////////////////////
    // group (groupname, createdat, description, meetingtimes)
    ////////////////////
    TABLE group {
        name groupname;
        time_point_sec createdat;
        string description;
        string meetingtimes;

        uint64_t primary_key() const { return groupname.value; }
    };

    typedef dapp::multi_index<"vgroup"_n, group> group_t;
    typedef eosio::multi_index<".vgroup"_n, group> group_t_v_abi;
    TABLE shardgroupbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vgroup"_n, shardgroupbucket> machine_t_abi;

    [[eosio::action]] void groupupser( name   p_groupname
                                     , string p_description
                                     , string p_meetingtimes
                                    ){
        group_t grouptable( _self, _self.value );
        auto group_iterator =  grouptable.find(p_groupname.value);
        if (group_iterator == grouptable.end()) {
            group_iterator  = grouptable.emplace(_self,  [&](auto& new_group) {
                new_group.groupname = p_groupname;
                new_group.createdat   = current_time_point_sec();
                new_group.description = p_description;
                new_group.meetingtimes= p_meetingtimes;
            });
        } else {// else... modify? (UPSERT?)
            grouptable.modify( group_iterator, /*eosio::same_payer*/_self, [&]( auto& edit_group ) {
                edit_group.groupname = p_groupname;
                edit_group.description = p_description;
                edit_group.meetingtimes= p_meetingtimes;
            });
        }
    }
    [[eosio::action]] void groupdelet( name   p_groupname ){
        group_t grouptable( _self, _self.value );
        auto group_iterator =  grouptable.find(p_groupname.value);
        if (group_iterator !=  grouptable.end()) {
            group_iterator  =  grouptable.erase(group_iterator);
        }
    }

    ////////////////////
    // hiscmach (machinename, createdat, playername,  machinestate, highscore)
    // hiscuser (playername,  createdat, machinename, machinestate, highscore)
    ////////////////////
    // since vRAM doesn't support multi-index, just primary index, we will use 2 vRAM tables.
    // we want to be able to fetch the high score by machine and by user, thus the names: HIgh_SCore_by_MACHine (hiscmach) and HIgh_SCore_by_USER (hiscuser).
    // they will be linked 1-to-1 and have a common UPSERT and DELETE actions, to keep them in sync.
    TABLE hiscmach {
        name hiscmachname;
        time_point_sec createdat;
        name ownername;
        name clubname;
        string serialnumber;

        uint64_t primary_key() const { return hiscmachname.value; }
    };

    typedef dapp::multi_index<"vhiscmach"_n, hiscmach> hiscmach_t;
    typedef eosio::multi_index<".vhiscmach"_n, hiscmach> hiscmach_t_v_abi;
    TABLE shardhiscmachbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vhiscmach"_n, shardhiscmachbucket> hiscmach_t_abi;
    // ---------------------------------
        TABLE hiscuser {
        name hiscusername;
        time_point_sec createdat;
        name ownername;
        name clubname;
        string serialnumber;

        uint64_t primary_key() const { return hiscusername.value; }
    };

    typedef dapp::multi_index<"vhiscuser"_n, hiscuser> hiscuser_t;
    typedef eosio::multi_index<".vhiscuser"_n, hiscuser> hiscuser_t_v_abi;
    TABLE shardhiscuserbucket {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"vhiscuser"_n, shardhiscuserbucket> hiscuser_t_abi;

    [[eosio::action]] void hiscupser( name   p_hiscname
                                       , name   p_ownername
                                       , name   p_clubname
                                       , string p_serialnumber
                                      ){
        // TODO: need to verify that the owner exists in the vAccounts table as a valid user. Also, check the club name in the vclub table.
        // need to verify foreign key, member, club...
        hiscuser_t hisctable( _self, _self.value );
        hiscmach_t hisctable( _self, _self.value );
        auto hiscmach_iterator =  hiscmachtable.find(p_hiscname.value);
        auto hiscuser_iterator =  hiscusertable.find(p_hiscname.value);
        if (hiscmach_iterator == hiscmachtable.end()) {
            hiscmach_iterator  = hiscmachtable.emplace(_self,  [&](auto& new_hisc) {
                // TODO: need to keep hiscmach in sync with hiscuser.
                new_hisc.hiscname = p_hiscname;
                new_hisc.createdat   = current_time_point_sec();
                new_hisc.ownername   = p_ownername;
                new_hisc.clubname    = p_clubname;
                new_hisc.serialnumber= p_serialnumber;
            });
        } else {// else... modify? (UPSERT?)
            hisctable.modify( hisc_iterator, /*eosio::same_payer*/_self, [&]( auto& edit_hisc ) {
                edit_hisc.hiscname = p_hiscname;
                edit_hisc.ownername   = p_ownername;
                edit_hisc.clubname    = p_clubname;
                edit_hisc.serialnumber= p_serialnumber;
            });
        }
    }
    [[eosio::action]] void hiscdelet( name   p_hiscname ){
        // TODO: find by user and machine and datetime? or just auto-increment uint64?
        hiscmach_t hiscmachtable( _self, _self.value );
        hiscuser_t hiscusesrtable( _self, _self.value );
        auto hiscmach_iterator =  hiscmachtable.find(p_hiscname.value);
        auto hiscuser_iterator =  hiscusertable.find(p_hiscname.value);
        if (hiscmach_iterator !=  hiscmachtable.end()) {
            hiscmach_iterator  =  hiscmachtable.erase(hisc_iterator);
        }
        if (hiscuser_iterator !=  hiscusertable.end()) {
            hiscuser_iterator  =  hiscusertable.erase(hisc_iterator);
        }
    }

    // Example interaction with this contract's tables using cleos and zeus to update different tables and check their content.
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

// CONTRACT_END((clubupsert)(clubdelete)(machineupser)(machinedelet)(withdraw)(hello)(hello2)(regaccount)(testschedule))
CONTRACT_END((clubupsert)(clubdelete)(machineupser)(machinedelet)(paymentinser)(paymentupdat)(paymentdelet)(withdraw)(regaccount)(testschedule))
