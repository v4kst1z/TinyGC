//
// Created by v4kst1z.
//

#include "../Tgc.h"

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

void test1() {
	HeapMap<int, std::string>* p = MakeGarbageCollected<HeapMap<int, std::string>>();
	p->insert(1, "aaa");
	p->insert(2, "bbb");
}
 

void test2() {
	HeapMap<LinkedNode, std::string>* p = MakeGarbageCollected<HeapMap<LinkedNode, std::string>>();
	for (int id = 0; id < 10; id++) {
		auto t = MakeGarbageCollected<LinkedNode>(nullptr, id);
		p->insert(t, "122");
	}
	
}

void test3() {
	HeapMap<int, LinkedNode>* p = MakeGarbageCollected<HeapMap<int, LinkedNode>>();
	for (int id = 0; id < 10; id++) {
		auto t = MakeGarbageCollected<LinkedNode>(nullptr, id);
		p->insert(id, t);
		//p->erase(id);
		std::cout << p->at(id) << std::endl;
	}
}

void test4() {
	HeapMap<LinkedNode, LinkedNode>* p = MakeGarbageCollected<HeapMap<LinkedNode, LinkedNode>>();
	for (int id = 0; id < 10; id++) {
		auto t1 = MakeGarbageCollected<LinkedNode>(nullptr, id);
		auto t2 = MakeGarbageCollected<LinkedNode>(nullptr, id + 100);

		p->insert(t1, t2);
		//p->erase(t1);
		std::cout << p->at(t1) << std::endl;
	}
}

int main() {
	tg.AttachMainThread();
	test1();
	test2();
	test3();
	test4();
	tg.DetachMainThread();
}