#pragma once
#include <yvals_core.h>
#include <chrono>
#include <memory>
#include <process.h>
#include <tuple>
#include <xthreads.h>
#include <mutex>

//using namespace std;

class BaseCounter
{
public:	
	virtual void increment() = 0;
	virtual int get() = 0;
	virtual void set(int value) = 0;
};

class AtomicCounter : BaseCounter
{
private:
	std::atomic<int> lockedvalue;
public:
	virtual void increment() override {
		lockedvalue++;
	}

	int get() override {
		return lockedvalue;
	}

	void set(int value) {
		lockedvalue = value;
	}

};

class Counter : BaseCounter
{
private:
	std::mutex mux;	
	//atomic<int> lockedvalue;
	int lockedvalue;
public:
	void increment() override {
		std::unique_lock<std::mutex> lm(mux);
		lockedvalue++;
	};

	int get() override {
		return lockedvalue;
	}

	void set(int value) override {
		std::unique_lock<std::mutex> lm(mux);
		lockedvalue = value;
	}
};

struct WorkerThread
{
	std::mutex mux;
	bool alive = true;
	bool mStarted = false;
	std::thread* thread=nullptr;
	BaseCounter* sharedCounter;
	std::condition_variable cvStart;
	std::function<void()> invokeable;

	WorkerThread(BaseCounter* aSharedCounter) {
		sharedCounter = aSharedCounter;
	}

	void Process()
	{
		while (alive)
		{
			std::unique_lock<std::mutex> lm(mux);
			if (mStarted) {
				cvStart.wait(lm);
			}
			mStarted = true;
			//Do activity
			invokeable();

			sharedCounter->increment();
		}
	}

	void Start()
	{
		std::unique_lock<std::mutex> lm(mux);
		if (!thread) {
			thread = new std::thread(&WorkerThread::Process, this);
		}
		cvStart.notify_one();
	}

};

class AbstractThreadPool
{
protected:
	virtual void processInvokeable(std::function<void()> invokeable) = 0;
public:

	template <class _Fn, class... _Args> void PushFunction(_Fn&& _Fx, _Args&&... _Ax) {
		std::function<void()> invokeable = [_Fx, _Ax...]() {std::invoke(_Fx, _Ax...); };
		processInvokeable(invokeable);
	};
	virtual void RunAll() =0;
	virtual void  WaitAll() = 0;
};

class CreateOnDemandThreadPool : public AbstractThreadPool
{
private:
	std::vector<std::thread> threads;
	virtual void processInvokeable(std::function<void()> invokeable) override
	{
		threads.push_back(std::thread( invokeable ));
	}

	virtual void RunAll() override
	{
		//Nothing done here
	}

	void WaitAll() override
	{
		for (int i = 0; i < threads.size(); i++) {
			if (threads[i].joinable()) {
				threads[i].join();
			}
		}
		threads.clear();
	}
};

class TPDThreadPool: public AbstractThreadPool
{
private:
	std::vector<WorkerThread*> workerThreads;
	int mMaxThreads = 0;
	int nCurrentThread = 0;
	BaseCounter* sharedCounter = (BaseCounter*) new AtomicCounter();

protected:
	virtual void processInvokeable(std::function<void()> invokeable) override 
	{
		if (workerThreads.size() == nCurrentThread) {
			workerThreads.push_back(new WorkerThread(sharedCounter));
		}
		workerThreads[nCurrentThread]->invokeable = invokeable;
		nCurrentThread++;
	}

public:
	TPDThreadPool(int numberOfThreads) {
		mMaxThreads = numberOfThreads;
	}

	virtual void RunAll() override
	{
		sharedCounter->set(0);
		for (int i = 0; i <= nCurrentThread - 1; i++) {
			workerThreads[i]->Start();
		}
	}

	void WaitAll() override
	{
		while (sharedCounter->get() < nCurrentThread) // Wait for all workers to complete
		{
			std::this_thread::yield();
		}
		nCurrentThread = 0;
	}
};
