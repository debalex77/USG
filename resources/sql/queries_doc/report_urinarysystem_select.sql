SELECT
    k.id,
    k.id_reportEcho,
    k.contour_right         AS kidney_contour_right,
    k.contour_left          AS kidney_contour_left,
    k.dimens_right          AS kidney_dimens_right,
    k.dimens_left           AS kidney_dimens_left,
    k.corticomed_right      AS kidney_corticomed_right,
    k.corticomed_left       AS kidney_corticomed_left,
    k.pielocaliceal_right   AS kidney_pielocaliceal_right,
    k.pielocaliceal_left    AS kidney_pielocaliceal_left,
    k.formations            AS kidney_formations,
    k.suprarenal_formations AS kidney_suprarenal_formations,
    k.concluzion            AS kidney_concluzion,
    k.recommendation        AS kidney_recommendation,
    b.volum                 AS bladder_volum,
    b.walls                 AS bladder_walls,
    b.formations            AS bladder_formations
FROM
    tableKidney k
LEFT JOIN
    tableBladder AS b ON b.id_reportEcho = k.id_reportEcho
WHERE
    k.id_reportEcho = :id_reports