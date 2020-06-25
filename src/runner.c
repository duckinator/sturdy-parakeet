#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Cardinals.h>

static String fallback_resources[] = {
    "*input:    True",
    "*showGrip: off",
    // regular slant; normal width; sans-serif; 20-29px; monospaced; unicode
    "*font:   -*-*-*-r-normal-sans-2?-*-*-*-m-*-iso10646-*",
    "*paned.orientation:    Horizontal",
    "?.?.text.preferredPaneSize:    500",
    "?.?.text.textSource.editType:  edit",
    NULL,
};


// Callback for when enter is pressed.
noreturn static void submit(Widget text_input) {
    Arg args[1];
    String command;

    XtSetArg(args[0], XtNstring, &command);
    XtGetValues(text_input, args, 1);

    pid_t pid = fork();
    if (pid == -1) { // failed to fork
        perror("runner");
        exit(1);
    }

    if (pid == 0) { // child process
        setsid();
        execlp("sh", "sh", "-c", command, NULL);
        exit(1);
    }
    // parent process does nothing
    exit(0);
}

// Callback for when X events happen.
static void ActionHook(Widget widget, XtPointer client_data, String action,
    XEvent *event, String *params, Cardinal *num_params) {

    // unused arguments.
    (void)client_data; (void)event; (void)params; (void)num_params;

    if (strcmp(action, "newline") == 0) {
        submit(widget);
    }
}

int main(int argc, char **argv) {
    XtAppContext app_context;
    Widget window;
    Widget paned;
    Widget text_input;

    window = XtAppInitialize(&app_context, "Xtext", NULL, 0,
            &argc, argv, fallback_resources,
            NULL, 0);

    paned = XtCreateManagedWidget("paned", panedWidgetClass, window, NULL, 0);
    text_input = XtCreateManagedWidget("text", asciiTextWidgetClass, paned, NULL, 0);

    XtSetKeyboardFocus(window, text_input);

    XtAppAddActionHook(app_context, ActionHook, NULL);

    XtRealizeWidget(window);
    XtAppMainLoop(app_context);
}
