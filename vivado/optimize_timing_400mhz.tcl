# ================================================================================
# Optimisations avancées de timing Vivado - cible 400 MHz
# ================================================================================

# Retiming de synthèse.
set_property STEPS.SYNTH_DESIGN.ARGS.MORE_OPTIONS {-retiming} [get_runs synth_1]

# Stratégies d’implémentation orientées performance.
set_property STRATEGY Performance_ExplorePostRoutePhysOpt [get_runs impl_1]
set_property STEPS.PLACE_DESIGN.ARGS.DIRECTIVE Explore [get_runs impl_1]
set_property STEPS.ROUTE_DESIGN.ARGS.DIRECTIVE PerformanceExplore [get_runs impl_1]

# Synthèse et implémentation.
launch_runs synth_1 -jobs 8
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1

# Rapport des chemins critiques et du slack à 400 MHz.
report_timing_summary -max_paths 10 -file timing_400mhz_report.txt
