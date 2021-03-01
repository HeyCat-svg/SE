#ifndef ydb_server_2pl_h
#define ydb_server_2pl_h

#include <string>
#include <map>
#include <vector>
#include "extent_client.h"
#include "lock_protocol.h"
#include "lock_client.h"
#include "lock_client_cache.h"
#include "ydb_protocol.h"
#include "ydb_server.h"

#define MAX_TRANS 1024

enum gntype {LOCKNODE, TRANSNODE};

typedef struct cache_entry {
	extent_protocol::extentid_t eid;
	std::string value;
	bool write_flag;

	cache_entry(extent_protocol::extentid_t eid, std::string value, bool write_flag=false){
		this->eid = eid;
		this->value = value;
		this->write_flag = write_flag;
	}
} cache_entry_t;

typedef struct gnode {
	gntype type;
	ydb_protocol::transaction_id trans_id;	// no this entry in lock node
	std::vector<cache_entry_t> cache;
	gnode *next;

	gnode(){
		type = LOCKNODE;
		trans_id = -1;
		next = NULL;
	}
} gnode_t;

class ydb_server_2pl: public ydb_server {
private:
	ydb_protocol::transaction_id trans_count;
	gnode_t lock_nodes[1024];
	gnode_t trans_nodes[MAX_TRANS];

	void add_edge(gnode_t *dst, gnode_t *src);
	void del_edge(gnode_t *node);
	bool find_circle(gnode_t *node);
	void put_trans(ydb_protocol::transaction_id trans_id);
	void del_trans(ydb_protocol::transaction_id trans_id);
	gnode_t *find_trans(ydb_protocol::transaction_id trans_id);

public:
	ydb_server_2pl(std::string, std::string);
	~ydb_server_2pl();
	ydb_protocol::status transaction_begin(int, ydb_protocol::transaction_id &);
	ydb_protocol::status transaction_commit(ydb_protocol::transaction_id, int &);
	ydb_protocol::status transaction_abort(ydb_protocol::transaction_id, int &);
	ydb_protocol::status get(ydb_protocol::transaction_id, const std::string, std::string &);
	ydb_protocol::status set(ydb_protocol::transaction_id, const std::string, const std::string, int &);
	ydb_protocol::status del(ydb_protocol::transaction_id, const std::string, int &);
};

#endif

