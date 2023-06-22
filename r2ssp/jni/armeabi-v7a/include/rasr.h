/** rasr.h
 
 Automatic Speech Recognition API
 Written by Gao Peng, 2016.12.28

 version 0.1: initial version.
 version 0.2: 2017.03.23, add grammar management APIs.
 version 0.2.1: 2017.04.11, add grammar prefix.
 version 0.2.2: 2017.04.12, add 'replace' parameters.
 version 0.2.3: 2017.04.13, add grammar compilation state check.
 version 0.2.4: 2017.05.19, add grammar domain compilation state check.
 version 0.3: 2017.06.03, add nbest and temp result support.
 version 0.3.1: 2017.06.05, add asr parameters about temp result, nbest, stat.
 version 0.3.2: 2017.06.12, add asr parameters about wfst output label result.
 version 0.4: 2017.07.21, 
    add asr parameters of auto stop/restart and sync recognition.
    add sub-pattern intent support.
 version 0.4.1: 2017.08.04,
    support output wfst ilabel/olabel both.
    add confirm-pattern intent support.
    get current recognition time.
    vad end parameter.
 version 0.4.2: 2017.09.01,
    add vad start detection.
    add cancel parameter for rec_stop.
 */

#ifndef RASR_H
#define RASR_H


#ifdef __cplusplus
extern "C" {
#endif


    /**
     Initialize asr module.
     return: 0 success, other value means error.
     */
    int rasr_init(const char *ini_file);

    /**
     Exit asr module. Close all asr lines before calling this.
     return: 0 success, other value means error.
     */
    int rasr_exit();


    /**
     ========================================================================
     Grammar management APIs.
     */

    /**
     Add a grammar item.
        domain: domain name the item belongs to.
        name: item name.
        value_num: how many values in this item.
        values: value array.
        replace: if true, replace all existing values in this item.
            if value_num is 0 and replace is true, this item will be deleted.
     return: 0 success, other value means error.
     */
    int rasr_grammar_add_item(const char *domain, const char *name,
                              int value_num, const char *values[], int replace);

    /**
     Add a grammar intent.
        domain: domain name the item belongs to.
        name: intent name.
        pattern_num: how many patterns in this item.
        patterns: pattern array.
        replace: if true, replace all existing patterns of pattern_type in 
            this intent. if pattern_num is 0 and replace is true and 
            pattern_type is 0, this intent will be deleted.
        pattern_type:
            if 0, patterns are normal patterns.
            if 1, patterns are sub-patterns.
            if 2, patterns are confirm-patterns.
     return: 0 success, other value means error.
     */
    int rasr_grammar_add_intent(const char *domain, const char *name,
                                int pattern_num, const char *patterns[],
                                int replace, int pattern_type);

    /**
     Set prefix grammar to all intents.
     Any accepted prefix can't be a prefix of another accepted prefix.
        prefix_num: how many prefixes.
        prefixes: prefix array.
     return: 0 success, other value means error.
     */
    int rasr_grammar_set_prefixes(int prefix_num, const char *prefixes[]);

    /**
     Delete all intents and items in a domain.
        name: domain name to delete.
            if name is '*', all domains will be deleted.
     return: 0 success, other value means error.
     */
    int rasr_grammar_delete_domain(const char *name);

    /**
     Compile grammars. Grammars must be compiled before take effect.
        sync: This step may take a long time to finish.
            The call will return immediately if sync is False.
     return: 0 success, other value means error.
     */
    int rasr_grammar_compile(int sync);

    /**
     Add a user customized grammar item. This item must already exists.
        user: user name.
        domain: domain name the item belongs to.
        name: item name.
        value_num: how many values in this item.
        values: value array.
        replace: if true, replace all existing user values in this item.
     return: 0 success, other value means error.
     */
    int rasr_grammar_set_user_item(const char *user, const char *domain, const char *name,
                                   int value_num, const char *values[], int replace);

    /**
     Set user customized prefix grammar to all intents.
     Any accepted prefix can't be a prefix of another accepted prefix.
        user: user name.
        prefix_num: how many prefixes.
        prefixes: prefix array.
     return: 0 success, other value means error.
     */
    int rasr_grammar_set_user_prefixes(const char *user, int prefix_num,
                                       const char *prefixes[]);

    /**
     Compile user customized grammars. Grammars must be compiled before take effect.
        sync: This step may take a long time to finish.
            The call will return immediately if sync is False.
     return: 0 success, other value means error.
     */
    int rasr_grammar_compile_user(const char *user, int sync);

    /**
     Check this user or system wide grammar compilation state.
        user: which user grammar compilation state to check.
            if null, check the system wide grammar compilation state.
     return: 0 not compiling, 1 compiling, other value means error.
     */
    int rasr_grammar_is_compiling(const char *user);

    /**
     Check domain/item grammar compilation state.
        domain: which domain compilation state to check.
        intent: which intent compilation state to check.
            if null, check the whole domain state.
     return: 0 not compiling, 1 compiling, other value means error.
     */
    int rasr_grammar_is_domain_compiling(const char *domain, const char *intent);

    /**
     ========================================================================
     Speech recognition APIs.
     */

    typedef void *RASR_LINE;
    typedef void *RASR_USERDATA;

    typedef struct _RASR_NOTIFY {
        int  notify_type;
        int  data;
        void *reserved;
    } RASR_NOTIFY;

    // notify type
    #define RASR_NOTIFY_RESULT          1
    #define RASR_NOTIFY_TMP_RESULT      2
    #define RASR_NOTIFY_ERROR          -1

    typedef int (*RASR_FN_RESULT_NOTIFY)(RASR_LINE line, RASR_NOTIFY *notify, RASR_USERDATA user_data);


    /**
     Create an asr line.
     return: asr line handle, NULL means error.
     */
    RASR_LINE rasr_create_line(RASR_FN_RESULT_NOTIFY result_notify,
                               RASR_USERDATA user_data);

    /**
     Close an asr line. Make sure the line is not working (rasr_rec_is_working).
     return: 0 success, other value means error.
     */
    int rasr_close_line(RASR_LINE line);


    /*
     The following parameters are used by rasr_set_param / rasr_get_param 
     to control asr line settings.
     */
    /*
     return temp reco result during recognition. default is off.
     temp result is the result from beginning, and may change over time.
     no guarantee to be prefix-no-change by default.
     param_value: int, 1/0, turn on/off temp reco result.
     */
    #define RASR_PARAM_RET_TEMP_RESULT          1
    /*
     return how many nbest reco result. default is 1.
     param_value: int, nbest result number.
     */
    #define RASR_PARAM_NBEST_RESULT_NUMBER      2
    /*
     return result with additional stat info. default is off.
     param_value: int, 1/0, turn on/off stat result.
     */
    #define RASR_PARAM_RESULT_WITH_STAT         3
    /*
     return result from wfst output label. default is 1.
        0: return am output label (also wfst input label) result.
        1: return wfst output label result.
        2: return both.
            format: "ilabel1/olabel1 ilabel2/ /olabel2 ilabel3/olable3 ..."
     param_value: int, 0/1/2.
     */
    #define RASR_PARAM_RET_WFST_RESULT          4
    /*
     recognizer will stop automatically if too many blanks. default is off.
     the leading blanks don't trigger auto stop.
     param_value: int, 1/0, turn on/off auto stop.
     */
    #define RASR_PARAM_AUTO_STOP                5
    /*
     recognizer will restart after auto stop. default is off.
     this will not work is auto stop is off.
     param_value: int, 1/0, turn on/off auto restart.
     */
    #define RASR_PARAM_AUTO_RESTART             6
    /*
     rec_input_waves/rec_stop will work in synchronized mode. default is off.
     param_value: int, 1/0, turn on/off synchronized recognition.
     */
    #define RASR_PARAM_SYNC_REC                 7
    /*
     vad end length (milliseconds) if auto stop is on.
     */
    #define RASR_PARAM_VAD_END_MS               8
    /*
     do vad on input speech. default is off (all input speech is recognized).
     param_value: int, 1/0, turn on/off vad.
     */
    #define RASR_PARAM_DO_VAD                   9
    /*
     vad begin length (milliseconds) if vad is on.
     */
    #define RASR_PARAM_VAD_BEGIN_MS             10


    /**
     Set asr line parameters. Extended parameters will be added in future.
     return: 0 success, other value means error.
     */
    int rasr_set_param(RASR_LINE line, int param, void *param_value);

    /**
     Get asr line parameters.
     */
    void *rasr_get_param(RASR_LINE line, int param);


    /**
     Select grammar domains and user grammars used for this line.
         domain_num=0 will clear grammar domains.
         domains="*" 
            select all grammar domains (only normal patterns).
         domains="domain_name"
            select all intents in a domain (only normal patterns in this domain)
         domains="domain_name+"
            select all intents in a domain (with sub-patterns in this domain)
         domains="domain_name/intent_name"
            select a specific intent (only normal patterns of this intent)
         domains="domain_name/intent_name+"
            select a specific intent (with sub-pattern of this intent)
         domains="domain_name/intent_name*"
            select a specific intent
                        (with sub-patterns and confirm-patterns of this intent)
         user = null will clear user grammars.
     return: 0 success, other value means error.
     */
    int rasr_select_grammar_domains(RASR_LINE line, int domain_num,
                                    const char *domains[], const char *user);

    /**
     Set grammars which will be compiled and enabled instantly.
     Grammar string format is a regular expression.
     grammar_num=0 will clear instant grammars.
     return: 0 success, other value means error.
     */
    int rasr_set_instant_grammars(RASR_LINE line, int grammar_num,
                                  const char *grammars[]);


    /**
     Start recognition.
     ac_model: acoustic model index, 0 default.
     lang_models: language model index,
        0 default (all)
        1 only general lm
     return: 0 success, other value means error.
     */
    int rasr_rec_start(RASR_LINE line, int ac_model, int lang_models);

    /**
     Input speech waves.
     If is_end is true, same as rasr_rec_stop (do not need call rasr_rec_stop again).
     return: 0 success, other value means error.
     */
    int rasr_rec_input_waves(RASR_LINE line, const short *inputs, int wave_num_shorts, int is_end);

    /**
     Stop recogintion on this line. If this line is in async mode (default), 
        using rasr_rec_is_working to check line status.
     cancel:
        0, all buffered speech will be reocgnized before stop;
        1, discard unprocessed speech and stop as soon as possible.
            It' possilbe that no RASR_NOTIFY_RESULT callback.
     return: 0 success, other value means error.
     */
    int rasr_rec_stop(RASR_LINE line, int cancel);

    /**
     Check if this asr line is in working state (rec_start has been called, not rec_stop;
         or rec_stoped, but not callback result).
     return: 0 not working, 1 working, other value means error.
     */
    int rasr_rec_is_working(RASR_LINE line);


    /**
     Get current nbest result number.
     return: nbest result number.
     */
    int rasr_get_result_number(RASR_LINE line);

    /**
     Get current recognition result length.
     nbest_index: which nbest result to get length.
     return: recognition result length.
     */
    int rasr_get_result_length(RASR_LINE line, int nbest_index);

    /**
     Get current recognition result.
     nbest_index: which nbest result to get result.
     return: 0 success, other value means error.
     */
    int rasr_get_result(RASR_LINE line, int nbest_index, char *result, int result_len);

    /**
     Get current recognition time (speech time length) since rec_start.
     return: recognition time in milli-seconds. -1 means error.
     */
    int rasr_get_rec_time(RASR_LINE line);


#ifdef __cplusplus
}
#endif

#endif /* RASR_H */
