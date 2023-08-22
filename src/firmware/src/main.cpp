#include "utilities/splash.h"
#include "utilities/assertions.h"
#include "task_manager/task_manager.h"

#define MASTER_CYCLE_TIME_MS 	1
#define MASTER_CYCLE_TIME_S 	(MASTER_CYCLE_TIME_MS * 1E-3)
#define MASTER_CYCLE_TIME_US 	(MASTER_CYCLE_TIME_MS * 1E3)
#define MASTER_CYCLE_TIME_ERR 	(MASTER_CYCLE_TIME_MS + 0.01)

FTYK timers;

// Runs once
void setup() {
	if (Serial) {
		splash();
	}
	timers.set(0);
	init_task_manager();
}

// Master loop
int main() {
	setup();

	timers.set(1);
	while (1) {
	
		spin();
	
		assert_leq<float>(timers.delay_millis(1, MASTER_CYCLE_TIME_MS), MASTER_CYCLE_TIME_ERR, "Teensy overcycled"); // timer error threashold (very tight)
		timers.set(1);
	}

	return 0;
}