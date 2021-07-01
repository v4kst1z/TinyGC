//
// Created by v4kst1z.
//

#include "../Tgc.h"
#include <queue>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

class LinkedNode final : public GarbageCollected<LinkedNode> {
public:
	LinkedNode(LinkedNode* next, int value) : next(next), value_(value) {}
	void Trace(Visitor* visitor) const override {
		visitor->Trace(next);
	}
private:
	Member<LinkedNode> next;
	int value_;
};


auto test() {
	HeapSet<LinkedNode>* vec = MakeGarbageCollected<HeapSet<LinkedNode>>();
	for (int id = 0; id < 10; id++) {
		auto p = MakeGarbageCollected<LinkedNode>(nullptr, id);
		vec->insert(p);
	}
	return vec;
}

int main() {
	tg.AttachMainThread();
	std::deque<int> tmp;
	std::unordered_map<int, int> ttt;
	ttt.insert({ 0, 0 });
	auto ret = test();
	//GarbageCollectedBase* x = ret->operator[](200); 
	ret->clear();
	HeapVector<LinkedNode>* vec = MakeGarbageCollected<HeapVector<LinkedNode>>();
	for (int id = 0; id < 10; id++) {
		auto p = MakeGarbageCollected<LinkedNode>(nullptr, id);
		vec->push_back(p);
	}
	LOG("test end");
	GarbageCollectedBase *p = MakeGarbageCollected<LinkedNode>(nullptr, 1);
	vec->push_back(dynamic_cast<LinkedNode *>(p));
	WorkThread t;
	auto res = t.Run([&]() {
		LinkedNode* p = MakeGarbageCollected<LinkedNode>(nullptr, 1);
		vec->push_back(p);
		int sum = 123;
		for (int id = 0; id < 1000; id++)
			sum += id;
		});
	t.join();
	WorkThread t1;
	auto res1 = t1.Run([]() { 
		int sum = 0;
		for (int id = 0; id < 1000; id++)
			sum += id;
		return sum; 
		});
	t1.join();
	tg.DetachMainThread();
	std::cout << "main end~" << std::endl;
}