#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <allegro5/allegro.h>

int running = 1;

void sigint_handler(int _) {
	running = 0;
}

int run_command(char* cmd) {
	//first, check if the process is running already
	DIR* procdir = opendir("/proc"); //assume the call wont return NULL... if it does.... theres probably something very wrong.
	struct dirent *ent;
	while ((ent = readdir (procdir)) != NULL) {
		int pid = atoi(ent->d_name);
		if (pid > 1000) { //pids < 1000 arent userspace
			char path[64];
			sprintf(path, "/proc/%i/cmdline", pid);
			int fd = open(path, O_RDONLY);
			char cmdline[64];
			read(fd, cmdline, strlen(cmd)+1);
			if (strcmp(cmd, cmdline) == 0) {
				//process running
				close(fd);
				closedir(procdir);
				return 0;
			}
			close(fd);
		}
	}
	closedir(procdir);

	system(cmd);
	return 1;
}

int main() {
	signal(SIGINT, sigint_handler);
	al_init();
	al_install_joystick();

	FILE* binds = fopen("binds.txt", "r");
	if (!binds) {
		printf("couldn't open binds file");
		return 1;
	}

	int button_combo[12];
	int button_combo_size = 0;
	int button_combo_strict = false; //if enabled, the combo must end with the correct button
	char combo_command[64] = "";

	while (!feof(binds) && !ferror(binds)) {
		int c = getc(binds);
		if (c == ',' || c == '?') {
			int b = 0;
			fscanf(binds, "%i", &b);
			button_combo[button_combo_size++] = b;
		} else if (c == '!') {
			button_combo_strict = true;
		} else if (c == '@') {
			fscanf(binds, "%s", combo_command);
		}
	}
	if (button_combo_size > 12) button_combo_size = 12;

	fclose(binds);

	ALLEGRO_JOYSTICK* joy = al_get_joystick(0);

	ALLEGRO_EVENT_QUEUE* event_queue;
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_joystick_event_source());

	printf("hello\n");

	while (running) {
		ALLEGRO_EVENT event;
		al_wait_for_event_timed(event_queue, &event, 10);
		if (joy && event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			int b = event.joystick.button;
			//printf("button %i\n", b);
			if (button_combo_strict ? b == button_combo[button_combo_size-1] : 1) {
				ALLEGRO_JOYSTICK_STATE st;
				al_get_joystick_state(joy, &st);
				for (int i = 0; i < button_combo_size; i++)
					if (!st.button[button_combo[i]]) goto no_match;
			} else goto no_match;

			//match:
			//system(combo_command);
			run_command(combo_command);
			
			//printf("you did the combo\n");
		} else if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
			//joystick may have been connected
			al_reconfigure_joysticks();
			joy = al_get_joystick(0);
		}

		continue;
		no_match:;
		//printf("huh??\n");
	}

	printf("goodbye\n");

	al_destroy_event_queue(event_queue);
	return 0;
}
