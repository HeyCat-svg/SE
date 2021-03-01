#ifndef ydb_server_occ_h
#define ydb_server_occ_h

#include <string>
#include <map>
#include <vector>
#include "extent_client.h"
#include "lock_protocol.h"
#include "lock_client.h"
#include "lock_client_cache.h"
#include "ydb_protocol.h"
#include "ydb_server.h"

#define MAX_TRANS_COUNT 1024

typedef struct cache_entry_occ {
	extent_protocol::extentid_t eid;
	std::string value;

	cache_entry_occ(extent_protocol::extentid_t eid, std::string value){
		this->eid = eid;
		this->value = value;
	}
} cache_entry_occ_t;


typedef struct trans_entry{
	ydb_protocol::transaction_id trans_id;
	std::vector<cache_entry_occ_t> read_set;
	std::vector<cache_entry_occ_t> write_set;

	trans_entry(){
		trans_id = -1;
	}
} trans_entry_t;


class ydb_server_occ: public ydb_server {
private:
	ydb_protocol::transaction_id trans_count;
	trans_entry_t trans_nodes[MAX_TRANS_COUNT];

	void put_trans(ydb_protocol::transaction_id trans_id);
	void del_trans(ydb_protocol::transaction_id trans_id);
	trans_entry *find_trans(ydb_protocol::transaction_id trans_id);

public:
	ydb_server_occ(std::string, std::string);
	~ydb_server_occ();
	ydb_protocol::status transaction_begin(int, ydb_protocol::transaction_id &);
	ydb_protocol::status transaction_commit(ydb_protocol::transaction_id, int &);
	ydb_protocol::status transaction_abort(ydb_protocol::transaction_id, int &);
	ydb_protocol::status get(ydb_protocol::transaction_id, const std::string, std::string &);
	ydb_protocol::status set(ydb_protocol::transaction_id, const std::string, const std::string, int &);
	ydb_protocol::status del(ydb_protocol::transaction_id, const std::string, int &);
};

#endif

