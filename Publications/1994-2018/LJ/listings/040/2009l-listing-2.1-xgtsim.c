/*
	xgtsim.c -- An XForms Game Theory Simulator
	
	On a "standard" Linux system, this program can be
	compiled with the command:
	
	gcc -lX11 -lforms -lm xgtsim.c -o xgtsim
	
	If that gives you problems, you are probably using a
	RedHat system. In that case, try:
	
	gcc -lforms -L/usr/X11R6/lib -lXpm -lX11 -lm xgtsim.c -o xgtsim

*/


/*
	We need a few include files, the most important of
	which is forms.h (which declares all the functions
	available in libforms)
*/

#include <forms.h>	/* XForms include file			*/
#include <stdlib.h>	/* Standard C stuff			*/
#include <time.h>	/* Need time to seed for random nubmers	*/

/*
	A few definitions
*/

#define		NUMB_TYPES	2	/* Two types of players */
#define		NUMB_PLAYERS	20	/* You can change this  
					   parameter to 1 if you 
					   want just one player 
					   of each type 	*/
#define		NUMB_ACTIONS	2	/* Two possible actions */
#define		NUMB_STATES	10	/* You can change this if
					   you want to allow more
					   complex (or simpler)
					   players		*/
#define		ACTION_A	0	/* Value to represent A	*/
#define		ACTION_B	1	/* Value to represent B	*/

/*
	Some global forms variables for the windows we'll be using
*/

FL_FORM		*main_window;
FL_FORM		*player_window;
FL_FORM		*payoff_window;
FL_FORM		*run_window;

/*
	We also need to make some forms objects global,
	since they are accessed by different functions
*/

FL_OBJECT	*action_choices[NUMB_STATES];
FL_OBJECT	*transition_inputs[NUMB_ACTIONS][NUMB_STATES];
FL_OBJECT	*payoff_inputs[NUMB_ACTIONS][NUMB_ACTIONS][NUMB_TYPES];
FL_OBJECT	*go_button;
FL_OBJECT	*stop_button;
FL_OBJECT	*column_chart;
FL_OBJECT	*row_chart;
FL_OBJECT	*column_browser;
FL_OBJECT	*row_browser;

/*
	Here are a few normal variables to store information
	about players, payoffs, and other parameters
*/

float	payoffs[NUMB_ACTIONS][NUMB_ACTIONS][NUMB_TYPES];
int 	state_actions[NUMB_TYPES][NUMB_STATES];
int	state_transitions[NUMB_TYPES][NUMB_ACTIONS][NUMB_STATES];
int	row_or_column;
int	numb_iterations;
int	abort_flag;

/*
	This routine sets some startup values
*/
void set_defaults()
{
	int i,j,k;
	time_t curtime;
	int seedval;
	/*
		(Pseudo-)randomly seed the (pseudo-)random number generator
	*/
	curtime = time(NULL);
	seedval = curtime;
	srand(seedval);
	/*
		This variable tells us whether changes in the player window
		should affect the column players or the row players
		Default is to edit column players
	*/
	row_or_column = 0;
	/*
		We'll do 1000 iterations of the game by default
	*/
	numb_iterations = 1000;
	/*
		Set Random Payoffs
	*/	
	for(i = 0; i < NUMB_ACTIONS; i++)
	{
		for(j=0; j < NUMB_ACTIONS; j++)
		{
			for(k = 0; k < NUMB_TYPES; k++)
			{
				payoffs[i][j][k] = (rand() % 1000) / 1000.0;
			}
		} 
	}
	/*
		Set random strategies and simple transitions
	*/
	for(i = 0; i < NUMB_TYPES; i++)
	{
		for(j = 0; j < NUMB_STATES; j++)
		{
			k = rand() % 1000;
			if(k < 500) state_actions[i][j] = ACTION_A;
			else state_actions[i][j] = ACTION_B;
			for(k = 0; k < NUMB_ACTIONS; k++)
			{
				state_transitions[i][k][j] = j+2;
				if(state_transitions[i][k][j] > NUMB_STATES)
					state_transitions[i][k][j] = 1; 
			}
		}
	}
}

/*
	The function set_player_values() is called whenever
	the user changes anything in the player window. Note
	that variable row_or_column is already set, so we
	only change the relevant values
*/
void set_player_values(FL_OBJECT *obj, long argument)
{
	int i;
	char a_string[10];
	for(i = 0; i < NUMB_STATES; i++)
	{
		if(fl_get_choice(action_choices[i]) == 1) 
			state_actions[row_or_column][i] = ACTION_A;
		else
			state_actions[row_or_column][i] = ACTION_B;
		state_transitions[row_or_column][0][i] = 
			atoi(fl_get_input(transition_inputs[0][i]));
		/*
			Here we check to make sure the user did not 
			specify a state value which is less than 1
			or greater than NUMB_STATES
		*/ 
		if(state_transitions[row_or_column][0][i] < 1)
		{
			state_transitions[row_or_column][0][i] = 1;
			sprintf(a_string, "1");
			fl_set_input(transition_inputs[0][i], a_string);
		}		
		if(state_transitions[row_or_column][0][i] > NUMB_STATES)
		{
			state_transitions[row_or_column][0][i] = NUMB_STATES;
			sprintf(a_string, "%d", NUMB_STATES);
			fl_set_input(transition_inputs[0][i], a_string);
		}		
		state_transitions[row_or_column][1][i] = 
			atoi(fl_get_input(transition_inputs[1][i]));		
		if(state_transitions[row_or_column][1][i] < 1)
		{
			state_transitions[row_or_column][1][i] = 1;
			sprintf(a_string, "1");
			fl_set_input(transition_inputs[1][i], a_string);
		}		
		if(state_transitions[row_or_column][1][i] > NUMB_STATES)
		{
			state_transitions[row_or_column][1][i] = NUMB_STATES;
			sprintf(a_string, "%d", NUMB_STATES);
			fl_set_input(transition_inputs[1][i], a_string);
		}		
	}
}

/*
	Here we read the values from the payoffs window
	into the payoffs array whenever a value is edited
	by the user
*/
void set_payoffs(FL_OBJECT *obj, long argument)
{
	int i,j,k;
	for(i = 0; i < NUMB_ACTIONS; i++)
	{
		for(j = 0; j < NUMB_ACTIONS; j++)
		{
			for(k = 0; k < NUMB_TYPES; k++)
			{
				payoffs[i][j][k] = atof(fl_get_input(payoff_inputs[i][j][k]));
			} 
		}
	}
	
}

/*
	This changes the number of iterations
*/
void set_iterations(FL_OBJECT *obj, long argument)
{
	numb_iterations = fl_get_counter_value(obj);
}

/*
	Whenever a button is pushed on the main window, this
	routine is called to make the relevant window appear
*/
void display_forms(FL_OBJECT *obj, long which_form)
{
	if(which_form == 1)
		fl_show_form(player_window,FL_PLACE_FREE,FL_FULLBORDER,"Player Design");
	if(which_form == 2)
		fl_show_form(payoff_window,FL_PLACE_FREE,FL_FULLBORDER,"Edit Payoffs");
	if(which_form == 3)
		fl_show_form(run_window,FL_PLACE_FREE,FL_FULLBORDER,"Play the Game");
}

/*
	close_forms() tells XForms that it's OK
	to close a window when if the window
	manager receives that signal (ie. if the
	user clicked on their window manager's
	close window icon
*/
int close_forms(FL_FORM *form, void *argument)
{
	/*
		We always want to let the user close windows
		If we wanted to prevent a window from being closed
		in this way, we could return FL_IGNORE instead
	*/
	return(FL_OK);
}


/*
	Are we editing row or column players
	in the players window?
*/
void set_row_or_column(FL_OBJECT *obj, long argument)
{
	int i;
	char a_string[10];
	/*
		Set row_or_column
	*/
	row_or_column = argument;
	/*
		Now update all the objects in the
		player window so that they show
		correct values
	*/
	for(i = 0; i < NUMB_STATES; i++)
	{
		if(state_actions[row_or_column][i] == ACTION_A) 
			fl_set_choice_text(action_choices[i], "A");
		else 
			fl_set_choice_text(action_choices[i], "B");
		sprintf(a_string,"%d", state_transitions[row_or_column][0][i]);
		fl_set_input(transition_inputs[0][i], a_string);
		sprintf(a_string,"%d", state_transitions[row_or_column][1][i]);
		fl_set_input(transition_inputs[1][i], a_string);
	}
}

/*
	Shuts down the program
*/
void quit_xgtsim(FL_OBJECT *obj, long argument)
{
	fl_finish();
	exit(0);
}


/*
	When the Stop! button is pushed, we set 
	this flag to 1.
*/
void stop_the_game(FL_OBJECT *obj, long argument)
{
	abort_flag = 1;
}

/*
	This routine runs the game by matching
	players, and updates information in the
	run window
*/
void play_the_game(FL_OBJECT *obj, long argument)
{
	int i,j,k,l;
	float column_average, row_average;
	char a_string[256];
	double smallest_payoff, largest_payoff;
	int to_match[NUMB_PLAYERS];
	int column_action;
	int row_action;
	int which_player, players_left;
	int current_state[NUMB_TYPES][NUMB_PLAYERS];
	float current_payoff[NUMB_TYPES][NUMB_PLAYERS];
	/*
		Start all players off in a random state
	*/
	for(i = 0; i < NUMB_TYPES; i++)
	{
		for(j = 0; j < NUMB_PLAYERS; j++)
		{
			current_state[i][j] = rand() % NUMB_STATES;
		}
	}
	/*
		Turn off the abort flag, then make the stop
		button appear so the user can abort the run
	*/	
	abort_flag = 0;
	fl_hide_object(go_button);
	fl_show_object(stop_button);
	/*
		Clear the charts
	*/
	fl_clear_chart(column_chart);
	fl_clear_chart(row_chart);
	/*
		Run the simulation
	*/
	for (i = 0; i < numb_iterations; i++)
	{
		/*
			Each time through, we want to scale the charts 
		*/
		smallest_payoff = payoffs[0][0][0];
		largest_payoff = payoffs[0][0][0];
		for(j = 0; j < NUMB_ACTIONS; j++)
		{
			for(k = 0; k < NUMB_ACTIONS; k++)
			{
				for(l = 0; l < NUMB_TYPES; l++)
				{
					if(smallest_payoff > payoffs[j][k][l])
						smallest_payoff = payoffs[j][k][l];
					if(largest_payoff < payoffs[j][k][l])
						largest_payoff = payoffs[j][k][l];
				}
			}
		}
		smallest_payoff = smallest_payoff - 0.05 * (largest_payoff - smallest_payoff);
		largest_payoff = largest_payoff + 0.05 * (largest_payoff - smallest_payoff);
		fl_set_chart_bounds(column_chart, smallest_payoff, largest_payoff);
		fl_set_chart_bounds(row_chart, smallest_payoff, largest_payoff);
		/*
			Now we randomly match column players against row players,
			calculate their payoffs, and change their states according
			to their transitions
		*/
		for(j = 0; j < NUMB_PLAYERS; j++)
		{
			to_match[j] = j;
		}
		column_average = 0.0;
		row_average = 0.0;
		players_left = NUMB_PLAYERS;
		for(j = 0; j < NUMB_PLAYERS; j++)
		{
			which_player = rand() % players_left;
			column_action = state_actions[0][current_state[0][j]];
			row_action = state_actions[1][current_state[1][to_match[which_player]]];
			current_payoff[0][j] = 
					payoffs[state_actions[0][current_state[0][j]]] 
					[state_actions[1][current_state[1][to_match[which_player]]]]
					[0];
			column_average = column_average + current_payoff[0][j];
			current_payoff[1][to_match[which_player]] = 
					payoffs[state_actions[0][current_state[0][j]]] 
					[state_actions[1][current_state[1][to_match[which_player]]]]
					[1];
			row_average = row_average + current_payoff[1][to_match[which_player]];
			current_state[0][j] = state_transitions[0]
						[row_action]
						[current_state[0][j]] - 1;
			current_state[1][to_match[which_player]] = 
						state_transitions[1]
						[column_action]
						[current_state[1][to_match[which_player]]] - 1;
			for(k = which_player; k < players_left - 1; k++)
			{
				to_match[k] = to_match[k+1];
			}
			players_left--;
		}
		column_average = column_average / NUMB_PLAYERS;
		row_average = row_average / NUMB_PLAYERS;
		/*
			Here's where we update the display with the new information
			from the lastest round of the game
		*/
		fl_add_chart_value(column_chart, column_average, "", FL_RED);
		sprintf(a_string,"%f", column_average);
		fl_clear_browser(column_browser);
		fl_add_browser_line(column_browser,a_string);

		fl_add_chart_value(row_chart, row_average, "", FL_RED);
		sprintf(a_string,"%f", row_average);
		fl_clear_browser(row_browser);
		fl_add_browser_line(row_browser,a_string);
		/*
			Now call fl_check_forms() to see if any information
			has changed (in particular, was the stop button
			pushed?)
		*/
		fl_check_forms();
		if(abort_flag == 1)
		{
			i = numb_iterations;
		}
	}
	/*
		We're done this run, so hide the Stop! button
		and show the Go button
	*/
	fl_hide_object(stop_button);
	fl_show_object(go_button);
}

/*
	This routine creates all our windows/forms and all the
	graphical elements on them
*/

void create_forms()
{
	/* 
		The "obj" pointer is used to create elements
		on all the various forms. We also need a
		group object to handle groupings
	*/ 
	FL_OBJECT	*obj;
	FL_OBJECT	*group;
	int 		i,j,k;
	char		a_string[10];
	/*
		The main window is used to access all
		the other windows, so we set the callbacks
		to the display_forms() function (except for Quit).
	*/
	main_window = fl_bgn_form(FL_NO_BOX, 290, 50);
		obj = fl_add_box(FL_UP_BOX,0,0,290,50,"");
		obj = fl_add_button(FL_NORMAL_BUTTON,10,10,60,30,"Players");
			fl_set_object_callback(obj, display_forms, 1);
		obj = fl_add_button(FL_NORMAL_BUTTON,80,10,60,30,"Payoffs");
			fl_set_object_callback(obj, display_forms, 2);
		obj = fl_add_button(FL_NORMAL_BUTTON,150,10,60,30,"Run");
			fl_set_object_callback(obj, display_forms, 3);
		obj = fl_add_button(FL_NORMAL_BUTTON,220,10,60,30,"Quit");
			fl_set_object_callback(obj, quit_xgtsim, 1);
	fl_end_form();
	/*
		The Player Window allows the user to set
		up player actions and strategies
	*/
	player_window = fl_bgn_form(FL_NO_BOX, 270, 100 + 20 * NUMB_STATES);
		obj = fl_add_box(FL_UP_BOX,0,0,270,100 + 20 * NUMB_STATES,"");
		obj = fl_add_box(FL_UP_BOX,10,50,250,10,"");
		/*
			This loop creates 10 text labels (one for
			each state) on the player window
		*/
		for(i = 0; i < NUMB_STATES; i++)
		{
			sprintf(a_string,"%d", i+1);
			obj = fl_add_text(FL_NORMAL_TEXT,20,90 + i * 20,20,20,a_string);
				fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		}
		/*
			Now we label this column of state numbers
		*/
		obj = fl_add_text(FL_NORMAL_TEXT,10,70,40,20,"States");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		/*
			For each state, we need a choice object to select
			the action in that state
		*/
		for(i = 0; i < NUMB_STATES; i++)
		{
			action_choices[i] = fl_add_choice(FL_NORMAL_CHOICE,60,90 + i * 20,50,20,"");
				fl_set_object_boxtype(action_choices[i],FL_EMBOSSED_BOX);
				fl_addto_choice(action_choices[i],"A");
				fl_addto_choice(action_choices[i],"B");
				if(state_actions[0][i] == ACTION_A) fl_set_choice_text(action_choices[i], "A");
				else fl_set_choice_text(action_choices[i], "B");
				fl_set_object_callback(action_choices[i], set_player_values, 0);
		}
		/*
			Label this column of actions
		*/
		obj = fl_add_text(FL_NORMAL_TEXT,60,70,50,20,"Actions");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		/*
			We also need two lists of which states to jump
			to while agents are "playing" the game
		*/
		for(i = 0; i < NUMB_STATES; i++)
		{
			transition_inputs[0][i] = fl_add_input(FL_INT_INPUT,140,90 + i * 20,30,20,"");
				sprintf(a_string,"%d", state_transitions[0][0][i]);
				fl_set_input(transition_inputs[0][i], a_string); 
				fl_set_object_callback(transition_inputs[0][i], set_player_values, 0);
			transition_inputs[1][i] = fl_add_input(FL_INT_INPUT,210,90 + i * 20,30,20,"");
				sprintf(a_string,"%d", state_transitions[0][1][i]);
				fl_set_input(transition_inputs[1][i], a_string); 
				fl_set_object_callback(transition_inputs[1][i], set_player_values, 0);
		}
		obj = fl_add_text(FL_NORMAL_TEXT,120,70,70,20,"A Transitions");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,190,70,70,20,"B Transitions");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		/*
			Finally (for this form) we need two buttons to choose
			between column players and row players. Note that we set
			the button type to FL_RADIO button so that XForms will
			ensure that only one is selected at a time
		*/
  		group = fl_bgn_group();
			obj = fl_add_lightbutton(FL_RADIO_BUTTON,10,10,110,30,"Column Players");
				fl_set_button(obj, 1);
				fl_set_object_callback(obj, set_row_or_column, 0);
			obj = fl_add_lightbutton(FL_RADIO_BUTTON,150,10,110,30,"Row Players");
				fl_set_object_callback(obj, set_row_or_column, 1);
		fl_end_group();
	fl_end_form();
	/*
		When the user clicks on the close window button
		in the title bar, run the following
	*/
	fl_set_form_atclose(player_window, close_forms, 0);
	/*
		Now we create the window for editing payoffs
	*/
	payoff_window = fl_bgn_form(FL_NO_BOX, 260, 180);
		obj = fl_add_box(FL_UP_BOX,0,0,260,180,"");
		/*
			We have two types of players, each of
			which can play 1 of 2 actions, giving
			us four possible outcomes. For each outcome,
			we need a payoff for each type of player.
		*/
		for(i = 0; i < NUMB_ACTIONS; i++)
		{
			for(j =0; j < NUMB_ACTIONS; j++)
			{
				for(k=0; k < NUMB_TYPES; k++)
				{
					payoff_inputs[i][j][k] = fl_add_input(FL_FLOAT_INPUT, 110 + 70 * i, 60 + j * 50 + k * 20, 60, 20,"");
					sprintf(a_string,"%5.5f", payoffs[i][j][k]);
					fl_set_input(payoff_inputs[i][j][k], a_string);
					fl_set_object_callback(payoff_inputs[i][j][k], set_payoffs, 0);
				}
			}
		}
		/*
			Here we add some text so that the user understands
			what the values in all those input fields
			represent
		*/
		obj = fl_add_text(FL_NORMAL_TEXT,130,10,110,20,"Column Players");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,10,100,70,20,"Row Players");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,80,70,20,20,"A");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,130,30,20,20,"A");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,200,30,20,20,"B");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		obj = fl_add_text(FL_NORMAL_TEXT,80,130,20,20,"B");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_end_form();
	fl_set_form_atclose(payoff_window, close_forms, 0);
	/*
	 	The run window lets us set the
	 	number of iterations, start/stop the game, and gives
	 	us visual feedback on a running game.
	*/
	run_window = fl_bgn_form(FL_NO_BOX, 420, 180);
		obj = fl_add_box(FL_UP_BOX,0,0,420,180,"");
		/*
			We want a line graph for both column players and row
			players, so we create charts of type FL_LINE_CHART
		*/
		column_chart = fl_add_chart(FL_LINE_CHART,10,30,190,90,"Column Players");
			fl_set_object_lalign(column_chart,FL_ALIGN_TOP|FL_ALIGN_LEFT);
			fl_set_chart_maxnumb(column_chart, 100);
		row_chart = fl_add_chart(FL_LINE_CHART,220,30,190,90,"Row Players");
			fl_set_object_lalign(row_chart,FL_ALIGN_TOP|FL_ALIGN_LEFT);
			fl_set_chart_maxnumb(row_chart, 100);
		/*
			We also create two browsers to give us a place to
			display numerical feedback
		*/
		column_browser = fl_add_browser(FL_NORMAL_BROWSER, 130, 5, 70, 21,"");
		row_browser = fl_add_browser(FL_NORMAL_BROWSER, 340, 5, 70, 21,"");
		/*
			Now we add a counter to let us set the number of
			iterations for the game
		*/
		obj = fl_add_counter(FL_NORMAL_COUNTER,60,140,140,30,"Iterations");
			fl_set_object_lalign(obj,FL_ALIGN_LEFT);
			fl_set_counter_precision(obj, 0);
			fl_set_counter_bounds(obj, 1, 100000);
			fl_set_counter_step(obj, 1, 100);
			fl_set_counter_value(obj, numb_iterations);
			fl_set_object_callback(obj, set_iterations, 0);
		/*
			We need buttons to start (Go!) and stop (Stop!) the 
			game. we draw them on top of each other, but then hide
			the Stop! button. The Go! button starts the game 
			running by calling play_the_game(). In that routine, 
			we hide the Go! button and show the Stop! button.
		*/
		go_button = fl_add_button(FL_NORMAL_BUTTON,220,140,190,30,"Go!");
			fl_set_object_callback(go_button, play_the_game, 0);
		stop_button = fl_add_button(FL_NORMAL_BUTTON,220,140,190,30,"Stop!");
			fl_set_object_callback(stop_button, stop_the_game, 0);
			fl_hide_object(stop_button);
	fl_end_form();
	fl_set_form_atclose(run_window, close_forms, 0);
}



int main(int argc, char *argv[])
{
	
	/*
		The first call is to fl_initialize(), which
		sets up XForms and handles relevant command
		line options
	*/
	fl_initialize(&argc, argv,"xldlas", 0, 0);
	/*
		We call set_defaults() to assign initial payoffs 
		and strategies, then create_forms() sets up all
		the windows for the program (but doesn't make
		any of them actually appear on the display)
	*/
	set_defaults();	
	create_forms();
	/*
		Now we show the main window and pass control over
		to the user with fl_do_forms();
	*/
        fl_show_form(main_window,FL_PLACE_FREE,FL_FULLBORDER,"An XForms Game Theory Simulator");
        fl_do_forms();
	return(0);
}

