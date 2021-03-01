#include "ydb_server_occ.h"
#include "extent_client.h"
#include <assert.h>

//#define DEBUG 1
#define TRANS_COUNT_LOCK 1025
#define COMMIT_LOCK 1026

inline static unsigned int BKDHash(const std::string &str){
	const char *cstr = str.c_str();
	unsigned int hash = 0;
	while(unsigned int ch = (unsigned int)*(cstr++)){
        hash = hash * 131 + ch;
    }
    return hash % 1024;
}

ydb_server_occ::ydb_server_occ(std::string extent_dst, std::string lock_dst) : ydb_server(extent_dst, lock_dst) {
	trans_count = 0;
}

ydb_server_occ::~ydb_server_occ() {
}


ydb_protocol::status ydb_server_occ::transaction_begin(int, ydb_protocol::transaction_id &out_id) {    // the first arg is not used, it is just a hack to the rpc lib
	// lab3: your code here
	lc->acquire(TRANS_COUNT_LOCK);
	out_id = (trans_count++) % MAX_TRANS_COUNT;
	lc->release(TRANS_COUNT_LOCK);
	put_trans(out_id);
	printf("transaction begin %d\n", out_id);
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_occ::transaction_commit(ydb_protocol::transaction_id id, int &) {
	// lab3: your code here
	printf("T%d: transaction commit begin\n", id);
	bool abort_flag = false;
	printf("111\n");
	trans_entry_t *trans = find_trans(id);
	printf("T%d: %p\n", id, trans);
	if (!trans) {
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	printf("222\n");
	lc->acquire(COMMIT_LOCK);
	printf("333\n");
	int read_size = trans->read_set.size();
	for (int i = 0; i < read_size; ++i) {
		cache_entry_occ_t loc_entry = trans->read_set[i];
		std::string remote_val;
		ec->get(loc_entry.eid, remote_val);
		if(remote_val != loc_entry.value){
			abort_flag = true;
			break;
		}
	}
	printf("444 abort_flag=%d\n", (int)abort_flag);
	if (!abort_flag){
		int write_size = trans->write_set.size();
		for (int i = 0; i < write_size; ++i) {
			cache_entry_occ_t loc_entry = trans->write_set[i];
			ec->put(loc_entry.eid, loc_entry.value);
		}
		del_trans(id);
		lc->release(COMMIT_LOCK);
		printf("T%d: transaction commit\n", id);
		return ydb_protocol::OK;
	}
	else{
		del_trans(id);
		lc->release(COMMIT_LOCK);
		printf("T%d: transaction ABORT ABNORMALLY\n", id);
		return ydb_protocol::ABORT;
	}
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_occ::transaction_abort(ydb_protocol::transaction_id id, int &) {
	// lab3: your code here
	trans_entry_t *trans = find_trans(id);
	if (!trans) {
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	del_trans(id);
	printf("T%d: transaction abort\n", id);
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_occ::get(ydb_protocol::transaction_id id, const std::string key, std::string &out_value) {
	// lab3: your code here
	// printf("T%d: get(%s) begin\n", id, key.c_str());
	extent_protocol::extentid_t eid = BKDHash(key);
	// printf("1\n");
	trans_entry_t *trans = find_trans(id);
	// printf("%p\n", trans);
	if (!trans) {
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	// find in cache first in write set then read set
	int write_size = trans->write_set.size();
	for (int i = 0; i < write_size; ++i) {
		if (trans->write_set[i].eid == eid) {
			out_value = trans->write_set[i].value;
			printf("T%d: get(%s) = %s\n", id, key.c_str(), out_value.c_str());
			return ydb_protocol::OK;
		}
	}
	int read_size = trans->read_set.size();
	for (int i = 0; i < read_size; ++i) {
		if (trans->read_set[i].eid == eid) {
			out_value = trans->read_set[i].value;
			printf("T%d: get(%s) = %s\n", id, key.c_str(), out_value.c_str());
			return ydb_protocol::OK;
		}
	}
	// first time to read
	ec->get(eid, out_value);
	// alloc new cache entry
	cache_entry_occ_t c(eid, out_value);
	trans->read_set.push_back(c);
	printf("T%d: get(%s) = %s\n", id, key.c_str(), out_value.c_str());
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_occ::set(ydb_protocol::transaction_id id, const std::string key, const std::string value, int &) {
	// lab3: your code here
	extent_protocol::extentid_t eid = BKDHash(key);
	trans_entry_t *trans = find_trans(id);
	if (!trans) {
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	int write_size = trans->write_set.size();
	for (int i = 0; i < write_size; ++i) {
		if (trans->write_set[i].eid == eid) {
			trans->write_set[i].value = value;
			printf("T%d: set(%s) = %s\n", id, key.c_str(), value.c_str());
			return ydb_protocol::OK;
		}
	}
	// first time to write
	cache_entry_occ_t c(eid, value);
	trans->write_set.push_back(c);
	printf("T%d: set(%s) = %s\n", id, key.c_str(), value.c_str());
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_occ::del(ydb_protocol::transaction_id id, const std::string key, int &) {
	// lab3: your code here
	extent_protocol::extentid_t eid = BKDHash(key);
	trans_entry_t *trans = find_trans(id);
	if (!trans) {
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	int write_size = trans->write_set.size();
	for (int i = 0; i < write_size; ++i) {
		if (trans->write_set[i].eid == eid) {
			trans->write_set[i].value = "";
			printf("T%d: del(%s)\n", id, key.c_str());
			return ydb_protocol::OK;
		}
	}
	// first time to write
	cache_entry_occ_t c(eid, "");
	trans->write_set.push_back(c);
	printf("T%d: del(%s)\n", id, key.c_str());
	return ydb_protocol::OK;
}

// custom function
void ydb_server_occ::put_trans(ydb_protocol::transaction_id trans_id){
	assert(trans_nodes[trans_id].trans_id == -1);
	trans_nodes[trans_id].trans_id = trans_id;
}

void ydb_server_occ::del_trans(ydb_protocol::transaction_id trans_id){
	trans_nodes[trans_id].trans_id = -1;
	trans_nodes[trans_id].read_set.clear();
	trans_nodes[trans_id].write_set.clear();
}

trans_entry_t *ydb_server_occ::find_trans(ydb_protocol::transaction_id trans_id){
	if (trans_id < 0 || trans_nodes[trans_id].trans_id == -1){
		return NULL;
	}
	else {
		return &(trans_nodes[trans_id]);
	}
}
