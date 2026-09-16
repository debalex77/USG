SELECT
    rp.docPresentationDate AS title_report,
    /* ----------------- liver ----------------- */
    %l_left%             AS liver_left,
    %l_right%            AS liver_right,
    tl.contur            AS liver_contur,
    tl.parenchim         AS liver_parenchim,
    tl.ecogenity         AS liver_ecogenity,
    tl.formations        AS liver_formations,
    tl.ductsIntrahepatic AS liver_duct_hepatic,
    tl.porta             AS liver_porta,
    tl.lienalis          AS liver_lienalis,
    /* ----------------- colecist ----------------- */
    tc.form           AS cholecist_form,
    tc.dimens         AS cholecist_dimens,
    tc.walls          AS cholecist_walls,
    tc.formations     AS cholecist_formations,
    tc.choledoc       AS cholecist_choledoc,
    /* ----------------- pancreas ----------------- */
    tp.cefal          AS pancreas_cefal,
    tp.corp           AS pancreas_corp,
    tp.tail           AS pancreas_tail,
    tp.ecogency       AS pancreas_ecogenity,
    tp.texture        AS pancreas_parenchim,
    tp.formations     AS pancreas_formations,
    /* ----------------- spleen ----------------- */
    ts.dimens         AS spleen_dimens,
    ts.contur         AS spleen_contur,
    ts.parenchim      AS spleen_parenchim,
    ts.formations     AS spleen_formations,
    /* ----------------- intestine ----------------- */
    til.formations    AS intestinal_formations,
    /* ----------------- other --------------------- */
    tl.concluzion     AS organs_internal_concluzion,
    tl.recommendation AS recommendation,
    r.concluzion      AS concluzion
FROM
    reportEcho r
LEFT JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
LEFT JOIN
    tableLiver tl ON tl.id_reportEcho = r.id
LEFT JOIN
    tableIntestinalLoop til ON til.id_reportEcho = r.id
LEFT JOIN
    tableCholecist tc ON tc.id_reportEcho = r.id
LEFT JOIN
    tablePancreas tp ON tp.id_reportEcho = r.id
LEFT JOIN
    tableSpleen ts ON ts.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
