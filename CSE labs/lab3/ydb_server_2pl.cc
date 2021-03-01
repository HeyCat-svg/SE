#include "ydb_server_2pl.h"
#include "extent_client.h"

//#define DEBUG 1
#define LOCAL_LOCK 1025
#define TRANS_COUNT_LOCK 1026

inline static unsigned int BKDHash(const std::string &str){
	const char *cstr = str.c_str();
	unsigned int hash = 0;
	while(unsigned int ch = (unsigned int)*(cstr++)){
        hash = hash * 131 + ch;
    }
    return hash % 1024;
}

ydb_server_2pl::ydb_server_2pl(std::string extent_dst, std::string lock_dst) : ydb_server(extent_dst, lock_dst) {
	trans_count = 0;
}

ydb_server_2pl::~ydb_server_2pl() {
}

ydb_protocol::status ydb_server_2pl::transaction_begin(int, ydb_protocol::transaction_id &out_id) {    // the first arg is not used, it is just a hack to the rpc lib
	// lab3: your code here
	lc->acquire(TRANS_COUNT_LOCK); // lock local var
	out_id = (trans_count++) % MAX_TRANS;
	lc->release(TRANS_COUNT_LOCK);
	put_trans(out_id);
	printf("transaction begin %d\n", out_id);
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_2pl::transaction_commit(ydb_protocol::transaction_id id, int &) {
	// lab3: your code here
	printf("transaction commit %d\n", id);
	gnode_t *trans = find_trans(id);
	if(!trans){
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	lc->acquire(LOCAL_LOCK);
	int size = trans->cache.size();
	for (int i = 0; i < size; ++i){
		printf("3 i=%d size=%d\n", i, size);
		extent_protocol::extentid_t eid = trans->cache[i].eid;
		printf("3.1 eid=%d\n", eid);
		if(trans->cache[i].write_flag){
			printf("3.2 value=%s\n", trans->cache[i].value.c_str());
			ec->put(eid, trans->cache[i].value);
			printf("3.3\n");
		}
		printf("4 eid=%d\n", eid);
		lc->release(eid);
		del_edge(&lock_nodes[eid]);
		printf("5\n", i, size);
	}
	printf("6\n");
	del_trans(id);
	printf("7\n");
	lc->release(LOCAL_LOCK);
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_2pl::transaction_abort(ydb_protocol::transaction_id id, int &) {
	// lab3: your code here
	printf("transaction abort %d\n", id);
	gnode_t *trans = find_trans(id);
	if(!trans){
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	lc->acquire(LOCAL_LOCK);
	int size = trans->cache.size();
	for (int i = 0; i < size; ++i){
		extent_protocol::extentid_t eid = trans->cache[i].eid;
		lc->release(eid);
		del_edge(&lock_nodes[eid]);
	}
	del_trans(id);
	lc->release(LOCAL_LOCK);
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_2pl::get(ydb_protocol::transaction_id id, const std::string key, std::string &out_value) {
	// lab3: your code here
	printf("T%d: get(%s) begin\n", id, key.c_str());
	extent_protocol::extentid_t eid = BKDHash(key);
	gnode_t *trans = find_trans(id);
	if(!trans){
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	// find eid in cache of trans
	int size = trans->cache.size();
	for (int i = 0; i < size; ++i){
		if (trans->cache[i].eid == eid) {
			out_value = trans->cache[i].value;
			printf("T%d: get(%s) = %s\n", id, key.c_str(), out_value.c_str());
			return ydb_protocol::OK;
		}
	}
	// first time to read
	lc->acquire(LOCAL_LOCK);
	add_edge(&lock_nodes[eid], trans);
	bool dead_lock = find_circle(&lock_nodes[eid]);
	printf("\tdeadlock_flag=%d\n", (int)dead_lock);
	// deadlock
	if (dead_lock){
		for (int i = 0; i < size; ++i) {
			extent_protocol::extentid_t eid = trans->cache[i].eid;
			lc->release(eid);
			del_edge(&lock_nodes[eid]);
		}
		del_trans(id);
		lc->release(LOCAL_LOCK);
		return ydb_protocol::ABORT;
	}
	// no deadlock
	lc->release(LOCAL_LOCK);

	lc->acquire(eid);

	lc->acquire(LOCAL_LOCK);			// 此处还是要加lock
	del_edge(trans);
	add_edge(trans, &lock_nodes[eid]);	// 因为此可能先加后删了 （添加成功的前提是获得edi锁，所以不会产生竞争 个鬼！！！！！
	ec->get(eid, out_value);
	lc->release(LOCAL_LOCK);
	// alloc new cache entry
	cache_entry_t c(eid, out_value);
	trans->cache.push_back(c);
	printf("T%d: get(%s) = %s\n", id, key.c_str(), out_value.c_str());
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_2pl::set(ydb_protocol::transaction_id id, const std::string key, const std::string value, int &) {
	// lab3: your code here
	printf("T%d: set(%s)=%s begin\n", id, key.c_str(), value.c_str());
	extent_protocol::extentid_t eid = BKDHash(key);
	gnode_t *trans = find_trans(id);
	// printf("T%d set begin: T%d->next=%p lock[%d]->next=%p\n", id, id, trans->next, eid, lock_nodes[eid].next);
	if(!trans){
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	// find eid in cache of trans
	int size = trans->cache.size();
	for (int i = 0; i < size; ++i){
		if (trans->cache[i].eid == eid) {
			trans->cache[i].value = value;
			trans->cache[i].write_flag = true;
			printf("T%d: set(%s)=%s end\n", id, key.c_str(), value.c_str());
			return ydb_protocol::OK;
		}
	}
	// first time to write
	lc->acquire(LOCAL_LOCK);
	add_edge(&lock_nodes[eid], trans);
	// printf("T%d set mid: T%d->next=%p lock[%d]->next=%p\n", id, id, trans->next, eid, lock_nodes[eid].next);
	bool dead_lock = find_circle(&lock_nodes[eid]);
	printf("\tdeadlock_flag=%d\n", (int)dead_lock);
	// deadlock
	if (dead_lock){
		printf("1\n");
		for (int i = 0; i < size; ++i) {
			extent_protocol::extentid_t eid = trans->cache[i].eid;
			lc->release(eid);
			del_edge(&lock_nodes[eid]);
		}
		del_trans(id);
		lc->release(LOCAL_LOCK);
		printf("2\n");
		return ydb_protocol::ABORT;
	}
	// no deadlock
	lc->release(LOCAL_LOCK);

	printf("--T%d: waiting lock(%d)\n", id, eid);
	lc->acquire(eid);
	printf("--T%d: acquired lock(%d)\n", id, eid);

	lc->acquire(LOCAL_LOCK);
	del_edge(trans);
	add_edge(trans, &lock_nodes[eid]);
	lc->release(LOCAL_LOCK);
	// printf("T%d set end: T%d->next=%p lock[%d]->next=%p\n", id, id, trans->next, eid, lock_nodes[eid].next);
	cache_entry_t c(eid, value, true);
	trans->cache.push_back(c);
	printf("T%d: set(%s)=%s end\n", id, key.c_str(), value.c_str());
	return ydb_protocol::OK;
}

ydb_protocol::status ydb_server_2pl::del(ydb_protocol::transaction_id id, const std::string key, int &) {
	// lab3: your code here
	extent_protocol::extentid_t eid = BKDHash(key);
	gnode_t *trans = find_trans(id);
	if(!trans){
		printf("find_trans fault!\n");
		return ydb_protocol::TRANSIDINV;
	}
	// find eid in cache of trans
	int size = trans->cache.size();
	for (int i = 0; i < size; ++i){
		if (trans->cache[i].eid == eid) {
			trans->cache[i].value = "";
			trans->cache[i].write_flag = true;
			return ydb_protocol::OK;
		}
	}
	// first time to write
	lc->acquire(LOCAL_LOCK);
	add_edge(&lock_nodes[eid], trans);
	bool dead_lock = find_circle(&lock_nodes[eid]);
	// deadlock
	if (dead_lock){
		for (int i = 0; i < size; ++i) {
			extent_protocol::extentid_t eid = trans->cache[i].eid;
			lc->release(eid);
			del_edge(&lock_nodes[eid]);
		}
		del_trans(id);
		lc->release(LOCAL_LOCK);
		return ydb_protocol::ABORT;
	}
	// no deadlock
	lc->release(LOCAL_LOCK);

	lc->acquire(eid);

	lc->acquire(LOCAL_LOCK);
	del_edge(trans);
	add_edge(trans, &lock_nodes[eid]);
	lc->release(LOCAL_LOCK);
	cache_entry_t c(eid, "", true);
	trans->cache.push_back(c);
	return ydb_protocol::OK;
}

// custom function
void ydb_server_2pl::add_edge(gnode_t *dst, gnode_t *src){
	src->next = dst;
}

void ydb_server_2pl::del_edge(gnode_t *node){
	node->next = NULL;
}

bool ydb_server_2pl::find_circle(gnode_t *node){
	printf("\tfind circle: %p->%p", node, node->next);
	gnode_t *np = node;
	gnode_t *p = np->next;
	while ((p != NULL) && (p != np)){
		p = p->next;
		printf("->%p", p);
	}
	printf("\n");
	// -------------test------------------
	// for (int i = 1; i < 4; ++i){
	// 	ydb_protocol::transaction_id id = trans_nodes[i].trans_id;
	// 	printf("\tT%d=%p\tT%d->next=%p\n", id, &trans_nodes[i], id, trans_nodes[i].next);
	// }
	// for (int i = 97; i < 100; ++i){
	// 	printf("\tlock[%d]=%p\tlock[%d]->next=%p\n", i, &lock_nodes[i], i, lock_nodes[i].next);
	// }
	// ------------test end-------------
	if (p == NULL) {
		return false;
	}
	else{
	 	return true;	
	}
}

void ydb_server_2pl::put_trans(ydb_protocol::transaction_id trans_id){
	trans_nodes[trans_id].trans_id = trans_id;
	trans_nodes[trans_id].type = TRANSNODE;
}

void ydb_server_2pl::del_trans(ydb_protocol::transaction_id trans_id){
	trans_nodes[trans_id].trans_id = -1;
	trans_nodes[trans_id].next = NULL;
	trans_nodes[trans_id].cache.clear();
}

gnode_t *ydb_server_2pl::find_trans(ydb_protocol::transaction_id trans_id){
	if (trans_id < 0 || trans_nodes[trans_id].trans_id == -1){
		return NULL;
	}
	else{
		return &(trans_nodes[trans_id]);
	}
}
