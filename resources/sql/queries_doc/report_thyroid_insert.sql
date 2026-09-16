INSERT INTO tableThyroid (
    id_reportEcho,
    thyroid_right_dimens,
    thyroid_right_volum,
    thyroid_left_dimens,
    thyroid_left_volum,
    thyroid_istm,
    thyroid_ecostructure,
    thyroid_formations,
    thyroid_ganglions,
    concluzion,
    recommendation
) VALUES (
    :id_reportEcho,
    :thyroid_right_dimens,
    :thyroid_right_volum,
    :thyroid_left_dimens,
    :thyroid_left_volum,
    :thyroid_istm,
    :thyroid_ecostructure,
    :thyroid_formations,
    :thyroid_ganglions,
    :concluzion,
    :recommendation
)