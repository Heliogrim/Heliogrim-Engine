#include "Await.hpp"

void hg::await(ref<Signal> signal_) {
	signal_.wait();
}

void hg::await(ref<JoinCounter> counter_) {
	counter_.wait();
}

void hg::await(ref<Batch> batch_) {
	batch_.wait();
}
