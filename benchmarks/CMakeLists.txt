# benchmarks/CMakeLists.txt
# UME Benchmarks Suite using Google Benchmark

if(UME_BUILD_BENCHMARKS)
    add_executable(ume_benchmarks
        bench_mpsc_queue.cpp
        bench_epoch_rcu.cpp
        bench_string_interner.cpp
        bench_journal_writer.cpp
        bench_replay_dispatcher.cpp
        bench_event_analysis.cpp
        bench_scheduler_placement.cpp
        bench_optimizer_prefetch.cpp
        bench_m6_learning_prediction.cpp
    )

    target_link_libraries(ume_benchmarks
        PRIVATE
            ume_core
            benchmark::benchmark
            benchmark::benchmark_main
    )

    set_target_properties(ume_benchmarks PROPERTIES
        OUTPUT_NAME ume_benchmarks
    )
endif()
