#include <pebble.h>

Window *window;
TextLayer *text_layer;

//The dictation session pointer we'll be using
DictationSession *dictation_session;
//The string/character pointer that we will be essentially copying our transcription to
static char last_text[512];

//Gets a human readable dictation status of each of the dictation statuses
char *get_readable_dictation_status(DictationSessionStatus status){
    switch(status){
        case DictationSessionStatusSuccess:
            return "Success";
        case DictationSessionStatusFailureTranscriptionRejected:
            return "User rejected success";
        case DictationSessionStatusFailureTranscriptionRejectedWithError:
            return "User rejected error";
        case DictationSessionStatusFailureSystemAborted:
            return "Too many errors, UI gave up";
        case DictationSessionStatusFailureNoSpeechDetected:
            return "No speech, UI exited";
        case DictationSessionStatusFailureConnectivityError:
            return "No BT/internet connection";
        case DictationSessionStatusFailureDisabled:
            return "Voice dictation disabled";
        case DictationSessionStatusFailureInternalError:
            return "Internal error";
        case DictationSessionStatusFailureRecognizerError:
            return "Failed to transcribe speech";
    }
    return "Unknown";
}

//This is called when the dictation API has something for us (good or bad)
void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
    //It checks if it's all good and in the clear
    if(status == DictationSessionStatusSuccess) {
        //Prints the transcription into the buffer
        snprintf(last_text, sizeof(last_text), "%s", transcription);
        //Sets it onto the text layer
        text_layer_set_text(text_layer, last_text);
    }
    //Otherwise if it is crap
    else{
        static char failed_buff[128];
        //Prints why to the failed buffer
        snprintf(failed_buff, sizeof(failed_buff), "Transcription failed because:\n%s", get_readable_dictation_status(status));
        //Sets it onto the text layer
        text_layer_set_text(text_layer, failed_buff);
    }
}

//This is called when the select button is pressed
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    dictation_session = dictation_session_create(sizeof(last_text), dictation_session_callback, NULL);
    dictation_session_start(dictation_session);
}

//This config provider provides the app with information on what buttons to register to the window
void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

//This funciton is called when the window loads
void window_load(Window *window) {
    //Load the window layer and the bounds
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    //Create the textlayer and set it up, then add it onto the window
    text_layer = text_layer_create(GRect(0, 50, bounds.size.w, 150));
    text_layer_set_text(text_layer, "Press select to do a transcription");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

//This is called when the window unloads
void window_unload(Window *window) {
    text_layer_destroy(text_layer); //(destroys the text layer)
}

//Initializes the window for the app and sets its handlers, then pushes it onto the stack, calling window_load
void init() {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);
}

//Deinits the app/window
void deinit() {
    window_destroy(window);
}

//The main function, as seen in all C programs
int main() {
    init();
    app_event_loop();
    deinit();
}
