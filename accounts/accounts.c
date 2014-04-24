#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <libaccounts-glib/accounts-glib.h>
#include <libsignon-glib/signon-glib.h>

typedef struct _AuthContext AuthContext;
struct _AuthContext {
    AgManager *manager;
    char *service_name;
    AgAccountService *account_service;
    AgAuthData *auth_data;
    SignonAuthSession *session;
};

static void login_cb(GObject *source, GAsyncResult *result, gpointer user_data) {
    SignonAuthSession *session = (SignonAuthSession *)source;
    AuthContext *ctx = (AuthContext *)user_data;

    GError *error = NULL;
    GVariant *session_data = signon_auth_session_process_finish(session, result, &error);
    if (error != NULL) {
        fprintf(stderr, "Authentication failed: %s\n", error->message);
        g_error_free(error);
        return;
    }

    const char *access_token = NULL;
    g_variant_lookup(session_data, "AccesToken", "&s", &access_token);
    if (access_token != NULL) {
        printf("Got access token: %s\n", access_token);
    }
}

static void login(AuthContext *ctx) {
    ctx->auth_data = ag_account_service_get_auth_data(ctx->account_service);

    GError *error = NULL;
    ctx->session = signon_auth_session_new(
        ag_auth_data_get_credentials_id(ctx->auth_data),
        ag_auth_data_get_method(ctx->auth_data), &error);
    if (error != NULL) {
        fprintf(stderr, "Could not set up auth session: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
#if 0
    g_variant_builder_add(
        &builder, "{sv}",
        SIGNON_SESSION_DATA_UI_POLICY,
        g_variant_new_int32(SIGNON_POLICY_NO_USER_INTERACTION));
#endif

    signon_auth_session_process_async(
        ctx->session,
        ag_auth_data_get_login_parameters(
            ctx->auth_data, g_variant_builder_end(&builder)),
        ag_auth_data_get_mechanism(ctx->auth_data),
        NULL, /* cancellable */
        login_cb, ctx);
}

AuthContext *watch_for_service(const char *service_name) {
    AuthContext *ctx = g_new0(AuthContext, 1);
    ctx->manager = ag_manager_new();
    ctx->service_name = g_strdup(service_name);

    GList *account_services = ag_manager_get_enabled_account_services(ctx->manager);
    GList *tmp;
    for (tmp = account_services; tmp != NULL; tmp = tmp->next) {
        AgAccountService *acct_svc = AG_ACCOUNT_SERVICE(tmp->data);
        AgService *service = ag_account_service_get_service(acct_svc);
        if (!strcmp(service_name, ag_service_get_name(service))) {
            ctx->account_service = g_object_ref(acct_svc);
            break;
        }
    }
    g_list_foreach(account_services, (GFunc)g_object_unref, NULL);
    g_list_free(account_services);

    if (ctx->account_service != NULL) {
        login(ctx);
    }

    return ctx;
}

int main(int argc, char **argv) {
    AuthContext *ctx = watch_for_service("soundcloud");
    GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);
}
