#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define MAX_USERS 100
#define CSV_FILE "audit_trail.csv"

const double W_AMOUNT    = 2.85;
const double W_VELOCITY  = 3.40;
const double W_AUTH_FAIL = 2.10;
const double W_OFF_HOURS = 1.65;
const double MODEL_BIAS  = -4.20;

typedef struct {
    char user_id[16];
    time_t last_txn_time;
    int txn_count_window;
} UserProfile;

UserProfile user_table[MAX_USERS];
int user_count = 0;

UserProfile* get_user(const char *user_id) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_table[i].user_id, user_id) == 0) return &user_table[i];
    }
    if (user_count < MAX_USERS) {
        strcpy(user_table[user_count].user_id, user_id);
        user_table[user_count].last_txn_time = 0;
        user_table[user_count].txn_count_window = 0;
        return &user_table[user_count++];
    }
    return NULL;
}

double run_ai_inference(double norm_amount, double velocity, double auth_fails, double off_hours) {
    double z = MODEL_BIAS +
               (W_AMOUNT * norm_amount) +
               (W_VELOCITY * velocity) +
               (W_AUTH_FAIL * auth_fails) +
               (W_OFF_HOURS * off_hours);
    return 1.0 / (1.0 + exp(-z));
}

int main(void) {
    FILE *audit = fopen(CSV_FILE, "w");
    if (audit) {
        fprintf(audit, "TIMESTAMP,TXN_ID,USER_ID,AMOUNT,AI_PROB,DECISION,MODEL_SIGNAL,LATENCY_US\n");
        fclose(audit);
    }

    const char *users[] = {"USR_101", "USR_102", "USR_103", "USR_104", "USR_105"};
    int txn_seq = 6001;

    printf("========================================================================================\n");
    printf("   IBM Z LinuxONE (s390x) REAL-TIME AI-DRIVEN PAYMENT FRAUD DECISIONING ENGINE          \n");
    printf("========================================================================================\n");
    printf("%-10s | %-8s | %-8s | %-12s | %-8s | %-8s | %-20s | %-8s\n",
           "TIME", "TXN ID", "USER", "AMOUNT (Rs)", "AI PROB", "DECISION", "AI INFERENCE SIGNAL", "LATENCY");
    printf("----------------------------------------------------------------------------------------\n");

    while (1) {
        struct timespec t_start, t_end;
        clock_gettime(CLOCK_MONOTONIC, &t_start);

        time_t now = time(NULL);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&now));

        char txn_id[16];
        snprintf(txn_id, sizeof(txn_id), "TXN%d", txn_seq++);

        const char *user_id = users[rand() % 5];
        UserProfile *profile = get_user(user_id);

        double amount;
        int failed_pins = 0;
        int scenario = rand() % 5;

        if (scenario == 0) {
            amount = 80000.0 + (rand() % 30000);
            failed_pins = 1;
        } else if (scenario == 1) {
            amount = 4500.0;
            failed_pins = 3;
        } else {
            amount = 300.0 + (rand() % 2500);
            failed_pins = 0;
        }

        double norm_amount = amount / 50000.0;
        double velocity_flag = 0.0;

        if (profile) {
            if (profile->last_txn_time != 0 && (now - profile->last_txn_time) <= 10) {
                profile->txn_count_window++;
                if (profile->txn_count_window >= 2) velocity_flag = 1.0;
            } else {
                profile->txn_count_window = 1;
            }
            profile->last_txn_time = now;
        }

        double off_hours = (rand() % 10 == 0) ? 1.0 : 0.0;

        double ai_prob = run_ai_inference(norm_amount, velocity_flag, (double)failed_pins, off_hours);

        char decision[16];
        char ai_signal[64];

        if (ai_prob >= 0.75) {
            strcpy(decision, "DECLINE");
            strcpy(ai_signal, "HIGH_ANOMALY_VECTOR");
        } else if (ai_prob >= 0.40) {
            strcpy(decision, "REVIEW");
            strcpy(ai_signal, "UNCERTAIN_PREDICTION");
        } else {
            strcpy(decision, "APPROVE");
            strcpy(ai_signal, "BASELINE_LEGITIMATE");
        }

        clock_gettime(CLOCK_MONOTONIC, &t_end);
        long latency_us = (t_end.tv_sec - t_start.tv_sec) * 1000000L +
                          (t_end.tv_nsec - t_start.tv_nsec) / 1000L;

        audit = fopen(CSV_FILE, "a");
        if (audit) {
            fprintf(audit, "%s,%s,%s,%.2f,%.2f,%s,%s,%ld\n",
                    time_str, txn_id, user_id, amount, ai_prob, decision, ai_signal, latency_us);
            fclose(audit);
        }

        if (strcmp(decision, "DECLINE") == 0) {
            printf("\033[1;31m%-10s | %-8s | %-8s | Rs.%-9.2f | %-7.2f%% | %-8s | %-20s | %ld us\033[0m\n",
                   time_str, txn_id, user_id, amount, ai_prob * 100.0, decision, ai_signal, latency_us);
        } else if (strcmp(decision, "REVIEW") == 0) {
            printf("\033[1;33m%-10s | %-8s | %-8s | Rs.%-9.2f | %-7.2f%% | %-8s | %-20s | %ld us\033[0m\n",
                   time_str, txn_id, user_id, amount, ai_prob * 100.0, decision, ai_signal, latency_us);
        } else {
            printf("\033[1;32m%-10s | %-8s | %-8s | Rs.%-9.2f | %-7.2f%% | %-8s | %-20s | %ld us\033[0m\n",
                   time_str, txn_id, user_id, amount, ai_prob * 100.0, decision, ai_signal, latency_us);
        }

        fflush(stdout);
        sleep(2);
    }
    return 0;
}
